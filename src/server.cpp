#include "server.hpp"

using std::cout;
using std::cerr;
using std::endl;
using std::string;

Server::Server(unsigned short port, const std::string& _dict_filename, uint8_t _num, uint8_t _den, unsigned int _max_players)
	: shutdown_signal(false), status(Server::Status::RUNNING), thread(&Server::start, this, port, _dict_filename, _num, _den, _max_players)
{
}

Server::~Server()
{
	shutdown();
	block();
}

void Server::start(unsigned short port, const std::string& _dict_filename, uint8_t _num, uint8_t _den, unsigned int _max_players)
{
	sf::UdpSocket socket;
	if (port == 0 || socket.bind(port) != sf::Socket::Status::Done)
	{
		cerr << "\nError: bad listening port " << port;
		std::lock_guard<std::mutex> lock(status_lock);
		status = Status::ABORTED;
		return;
	}
	socket.setBlocking(false);

	sf::Clock timer;

	string peeler;

	std::vector<string> remove;

	Game game(_dict_filename, _num, _den, _max_players);

	cout << "\nWaiting for players to join...";
	cout.flush();
	while (true) // TODO consumes 100% CPU
	{
		float elapsed = timer.getElapsedTime().asSeconds();
		timer.restart();

		// send pending packets
		for (auto& pair : game.get_players())
		{
			if (pair.second.has_pending())
			{
				pair.second.step(elapsed);

				// check for timeout
				if (pair.second.get_timeout() > 5.f)
				{
					cout << endl << pair.second.get_name() << " timed out";
					cout.flush();

					remove.push_back(pair.first);
				}
				// else send pending packets
				else if (pair.second.poll > 0.5f)
				{
					pair.second.poll -= 0.5f;
					sf::Packet pending(pair.second.get_pending());
					socket.send(pending, pair.second.get_ip(), pair.second.get_port());
				}
			}
		}

		// handle timed out players
		if (!remove.empty())
		{
			// remove them from the game
			for (const auto& id : remove)
				game.remove_player(id);

			// notify other players
			for (const auto& id : remove)
			{
				// notify players of timeout
				sf::Packet leave;
				leave << sv_info << id << sf::Uint8(1);
				for (auto& pair : game.get_players())
				{
					if (!pair.second.has_pending())
						socket.send(leave, pair.second.get_ip(), pair.second.get_port());
					pair.second.add_pending(leave);
					game.wait();
				}
			}

			remove.clear();
		}

		// check if we got a shutdown signal
		bool need_to_shutdown;
		{
			std::lock_guard<std::mutex> lock(shutdown_lock);
			need_to_shutdown = shutdown_signal;
		}
		if (need_to_shutdown)
		{
			cout << "\nServer shutting down...";
			cout.flush();

			std::lock_guard<std::mutex> lock(status_lock);
			status = Status::ABORTED;

			if (game.get_players().size() > 0)
			{
				cout << "\nNotifying players...";
				cout.flush();

				for (const auto& pair: game.get_players())
				{
					sf::Packet sorry;
					sorry << sv_disconnect << sf::Uint8(4);
					socket.send(sorry, pair.second.get_ip(), pair.second.get_port());
				}
			}

			break;
		}

		// if the game has just ended
		if (game.is_ready_to_finish())
		{
			game.finish();

			sf::Packet win;
			win << sv_done;

			// if there is an actual winner
			if (game.get_players().size() > 1)
			{
				win << sf::Uint8(1) << game.winner;
				cout << endl << game.get_player_name(game.winner) << " won the game!";
			}
			else
			{
				win << sf::Uint8(0);
				cout << "\nGame cannot continue. Too many players left.";
			}
			cout.flush();

			// send victory notification
			for (auto& pair : game.get_players())
			{
				if (!pair.second.has_pending())
					socket.send(win, pair.second.get_ip(), pair.second.get_port());

				pair.second.add_pending(win);
				game.wait();
			}
		}

		if (game.can_shutdown())
		{
			cout << "\nShutting down server...";
			cout.flush();

			std::lock_guard<std::mutex> lock(status_lock);
			status = Status::DONE;

			break;
		}

		// get new packets
		sf::Packet packet;
		sf::IpAddress client_ip;
		unsigned short client_port;

		if (socket.receive(packet, client_ip, client_port) != sf::Socket::Status::Done)
			continue;

		sf::Uint8 type;
		std::string id;

		// every client packet starts with a type and player id
		packet >> type;
		packet >> id;

		// only process packets if we know the player or they're connecting
		if (type != cl_connect && !game.has_player(id))
			continue;

		switch(type)
		{
			case cl_connect:
			{
				sf::Uint8 version;
				packet >> version;
				if (version != protocol_version)
				{
					sf::Packet sorry;
					sorry << sv_disconnect << sf::Uint8(0);
					socket.send(sorry, client_ip, client_port);

					cout << "\nclient failed to join: "
					        "need protocol version " << (int)protocol_version << ", got " << (int)version;
					cout.flush();
					break;
				}

				std::string name;
				packet >> name;

				// if it is an unknown player
				if (!game.has_player(id))
				{
					if (game.is_full())
					{
						sf::Packet sorry;
						sorry << sv_disconnect << sf::Uint8(1);
						socket.send(sorry, client_ip, client_port);

						cout << "\nclient failed to join: game full";
						cout.flush();
						break;
					}

					if (game.in_progress())
					{
						sf::Packet sorry;
						sorry << sv_disconnect << sf::Uint8(3);
						socket.send(sorry, client_ip, client_port);

						cout << "\nclient failed to join: game already started";
						cout.flush();
						break;
					}

					// let everyone else know about the connection
					sf::Packet join;
					join << sv_info << id << sf::Uint8(0) << name;
					for (auto& pair : game.get_players())
					{
						if (!pair.second.has_pending())
							socket.send(join, pair.second.get_ip(), pair.second.get_port());
						pair.second.add_pending(join);
						game.wait();
					}

					Player& new_player = game.add_player(id, client_ip, client_port, name);

					// let new player know about all other players
					for (auto& pair : game.get_players())
					{
						if (pair.first == id) continue;

						sf::Packet join;
						join << sv_info << pair.first << sf::Uint8(0) << pair.second.get_name();
						if (!new_player.has_pending())
							socket.send(join, client_ip, client_port);
						new_player.add_pending(join);
						game.wait();
					}

					cout << "\n" << name << " has joined the game";
					cout.flush();
				}

				// by now we know they are a player, ack connection
				sf::Packet join;
				join << sv_info << id << sf::Uint8(0) << name;
				socket.send(join, client_ip, client_port);

				break;
			}
			case cl_disconnect:
			{
				cout << "\n" << game.get_player_name(id) << " has left the game";
				cout.flush();

				game.remove_player(id);

				// let everyone else know about disconnection
				sf::Packet leave;
				leave << sv_info << id << sf::Uint8(1);
				for (auto& pair : game.get_players())
				{
					if (!pair.second.has_pending())
						socket.send(leave, pair.second.get_ip(), pair.second.get_port());
					pair.second.add_pending(leave);
					game.wait();
				}

				break;
			}
			case cl_ready:
			{
				bool ready;
				packet >> ready;

				game.set_ready(id, ready);

				sf::Packet rdy;
				rdy << sv_info << id << (ready ? sf::Uint8(2) : sf::Uint8(3));
				for (const auto& pair : game.get_players())
					socket.send(rdy, pair.second.get_ip(), pair.second.get_port());

				break;
			}
			case cl_check:
			{
				string word;
				packet >> word;

				sf::Packet lookup;
				lookup << sv_check << word << game.check_word(word);

				socket.send(lookup, client_ip, client_port);

				break;
			}
			case cl_dump:
			{
				if (game.is_finished())
				{
					cout << "\nDump received after game end";
					cout.flush();
					break;
				}

				sf::Int16 dump_n;
				sf::Int8 chr;
				packet >> dump_n >> chr;

				if (game.check_dump(id, dump_n))
				{
					sf::Packet dump;

					string letters = game.dump(id, dump_n, chr);

					cout << endl << game.get_player_name(id) << " dumped " << chr
					     << " and received " << letters;
					cout.flush();

					dump << sv_dump << dump_n << letters;

					socket.send(dump, client_ip, client_port);
				}

				break;
			}
			case cl_peel:
			{
				if (game.is_finished())
				{
					cout << "\nPeel received after game end";
					cout.flush();
					break;
				}

				sf::Int16 client_peel;
				packet >> client_peel;

				// make sure peel number is correct
				if (game.check_peel(client_peel))
				{
					// store first peeler id
					if (peeler.size() == 0)
						peeler = id;
					else
					{
						cout << "\n" << game.get_player_name(id) << " also peeled. Waiting on some players...";
						cout.flush();
					}
				}
				else
				{
					cout << "\nPeel out of order: got " << (int)client_peel << ", expected " << (int)game.get_peel();
					cout.flush();
				}

				break;
			}
			case cl_ack:
			{
				sf::Int16 ack_num;
				packet >> ack_num;

				Player& player = game.get_players().at(id);

				// if we were waiting for this ACK
				if (player.acknowledged(ack_num))
				{
					// if they have more pending, send the first
					if (player.has_pending())
					{
						sf::Packet pending(player.get_pending());
						socket.send(pending, player.get_ip(), player.get_port());
					}
					else // check if game can proceed
						game.check_waiting();
				}
				else
				{
					cout << "\nMismatched ACK (" << (int)ack_num << ")";
					cout.flush();
				}
				break;
			}
			default:
				cout << "\nUnrecognized packet type: " << (int)type;
				cout.flush();
		}

		// peel
		if (game.can_peel())
		{
			cout << endl;
			if (peeler.size() == 0)
				cout << "Split!";
			else
				cout << game.get_player_name(peeler) << ": Peel!";

			// if victory
			if (game.peel())
				game.winner = peeler;
			else
			{
				sf::Int16 remaining = game.get_remaining();

				for (auto& pair : game.get_players())
				{
					std::string letters = pair.second.get_peel();

					cout << "\nSending " << pair.second.get_name() << " " << letters;

					sf::Packet peel;
					peel << sv_peel << sf::Int16(game.get_peel() - 1) << remaining << peeler << letters;

					if (!pair.second.has_pending())
						socket.send(peel, pair.second.get_ip(), pair.second.get_port());

					pair.second.add_pending(peel);
					game.wait();
				}
			}

			peeler.clear();

			cout.flush();
		}
	}
}

void Server::shutdown()
{
	std::lock_guard<std::mutex> lock(shutdown_lock);
	shutdown_signal = true;
}

Server::Status Server::block()
{
	{
		std::lock_guard<std::mutex> lock(status_lock);
		if (status != Status::RUNNING)
			return status;
	}

	thread.join();
	return status;
}

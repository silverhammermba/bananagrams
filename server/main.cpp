#include "server.hpp"

namespace po = boost::program_options;
using std::cout;
using std::cerr;
using std::endl;
using std::string;

sf::UdpSocket socket;
unsigned short client_port;

Game* game;

// callback for interrupt signal
void shutdown(int s)
{
	(void)s; // intentionally unused

	cout << "\nServer shutting down...";
	cout.flush();

	if (game != nullptr)
	{
		cout << "\nNotifying players...";
		for (const auto& pair: game->get_players())
		{
			sf::Packet sorry;
			sorry << sv_disconnect << sf::Uint8(4);
			socket.send(sorry, pair.second.get_ip(), client_port);
		}
		cout << endl;

		delete game;
	}

	exit(0);
}

int main(int argc, char* argv[])
{
	// set up shutdown callback for interrupts
	struct sigaction action;
	action.sa_handler = shutdown;
	sigemptyset(&action.sa_mask);
	action.sa_flags = 0;

	sigaction(SIGINT, &action, nullptr);
	sigaction(SIGTERM, &action, nullptr);
	sigaction(SIGKILL, &action, nullptr);

	// command line arguments
	po::options_description desc("Bananagrams dedicated server options");
	desc.add_options()
		("help",                                                                   "show options")
		("dict",  po::value<string>()->required(),                                 "dictionary file")
		("port",  po::value<unsigned short>()->default_value(default_server_port), "TCP/UDP listening port")
		("bunch", po::value<string>()->default_value("1"),                         "bunch multiplier (0.5 or a positive integer)")
		("limit", po::value<unsigned int>(),                                       "player limit")
	;

	// TODO usage string
	// TODO print help on unrecognized options
	po::variables_map opts;
	po::store(po::parse_command_line(argc, argv, desc), opts);

	if (opts.count("help"))
	{
		cerr << desc << endl;
		return 1;
	}

	try
	{
		po::notify(opts);
	}
	catch (po::required_option& e)
	{
		cerr << "Error: " << e.what() << endl << endl << desc << endl;
		return 1;
	}

	// check port option
	unsigned short server_port {opts["port"].as<unsigned short>()};
	if (server_port == 0)
	{
		cerr << "Invalid listening port: " << server_port << "!\n";
		return 1;
	}

	client_port = server_port + 1;

	// game options
	unsigned int b_num;
	unsigned int b_den;
	unsigned int max_players;
	std::map<string, string> dictionary;

	// load options from command line

	// check bunch multiplier option
	std::stringstream multi_s;
	multi_s << opts["bunch"].as<string>();
	b_num = 1;
	b_den = 1;
	if (multi_s.str() == "0.5")
		b_den = 2;
	else
		multi_s >> b_num;

	max_players = (8 * b_num) / b_den;

	if (opts.count("limit"))
	{
		auto limit = opts["limit"].as<unsigned int>();

		if (limit > max_players)
		{
			cerr << "\nmax player limit is " << max_players << " for this bunch size";
			cerr.flush();
		}
		else if (limit < 2)
		{
			cerr << "\nmin player limit is 2";
			cerr.flush();
		}
		else
		{
			max_players = limit;
		}
	}

	// check dictionary option
	if (!opts.count("dict"))
	{
		cerr << "No dictionary file specified!\n";
		return 1;
	}

	string dict = opts["dict"].as<string>();

	cout << "Loading dictionary... ";
	cout.flush();
	std::ifstream words(dict);
	if (!words.is_open())
	{
		std::cerr << "\nFailed to open " << dict << "!\n";
		return 1;
	}

	// parse dictionary
	string line;
	while (std::getline(words, line))
	{
		auto pos = line.find_first_of(' ');
		if (pos == string::npos)
			dictionary[line] = "";
		else
			dictionary[line.substr(0, pos)] = line.substr(pos + 1, string::npos);
	}
	words.close();
	cout << dictionary.size() << " words found";
	cout.flush();

	// TODO srand
	// LET'S GO!!!

	// TODO for debugging
	/*
	auto it = bunch.begin();
	for (unsigned int i = 0; i < 25; i++)
		++it;
	bunch.erase(it, bunch.end());
	*/

	// TODO catch failure
	socket.bind(server_port);

	sf::Clock timer;

	string peeler;

	game = new Game(b_num, b_den, max_players);

	cout << "\nWaiting for players to join...";
	cout.flush();
	while (true)
	{
		sf::Packet packet;
		sf::IpAddress client_ip;
		unsigned short client_port;
		socket.receive(packet, client_ip, client_port);

		cout << "\nReceived packet from " << client_ip << ":" << client_port;
		cout.flush();

		sf::Uint8 type;
		std::string id;

		// every client packet starts with a type and player id
		packet >> type;
		packet >> id;

		// only process packets if we know the player or they're connecting
		if (type != cl_connect && !game->has_player(id))
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

					cout << "\nclient failed to join"
						    "\n\tNeed protocol version " << (int)protocol_version << ", got " << (int)version;
					cout.flush();
					break;
				}

				std::string name;
				packet >> name;

				// if it is an unknown player
				if (!game->has_player(id))
				{
					if (game->is_full())
					{
						sf::Packet sorry;
						sorry << sv_disconnect << sf::Uint8(1);
						socket.send(sorry, client_ip, client_port);

						cout << "\nclient failed to join"
								"\n\tGame full";
						cout.flush();
						break;
					}

					if (game->in_progress())
					{
						sf::Packet sorry;
						sorry << sv_disconnect << sf::Uint8(3);
						socket.send(sorry, client_ip, client_port);

						cout << "\nclient failed to join"
								"\n\tGame already started";
						cout.flush();
						break;
					}

					// let everyone else know about the connection
					sf::Packet join;
					join << sv_info << id << sf::Uint8(0) << name;
					for (auto& pair : game->get_players())
					{
						if (!pair.second.has_pending())
							socket.send(join, pair.second.get_ip(), client_port);
						pair.second.add_pending(join);
						game->wait();
					}

					game->add_player(id, client_ip, name);

					cout << "\n" << name << " has joined the game";
					cout.flush();
				}

				// by now they're definitely a player, confirm connection
				sf::Packet join;
				join << sv_info << id << sf::Uint8(0) << name;
				socket.send(join, game->get_players().at(id).get_ip(), client_port);
				// TODO notify new players of existing players

				break;
			}
			case cl_disconnect:
			{
				cout << "\n" << game->get_player_name(id) << " has left the game";
				cout.flush();

				game->remove_player(id);

				// let everyone else know about disconnection
				sf::Packet leave;
				leave << sv_info << id << sf::Uint8(1);
				for (auto& pair : game->get_players())
				{
					if (!pair.second.has_pending())
						socket.send(leave, pair.second.get_ip(), client_port);
					pair.second.add_pending(leave);
					game->wait();
				}

				// TODO end game if now below limit

				break;
			}
			case cl_ready:
			{
				bool ready;
				packet >> ready;

				game->set_ready(id, ready);

				sf::Packet rdy;
				rdy << sv_info << id << (ready ? sf::Uint8(2) : sf::Uint8(3));
				for (const auto& pair : game->get_players())
					socket.send(rdy, pair.second.get_ip(), client_port);

				break;
			}
			case cl_check:
			{
				string word;
				packet >> word;

				sf::Packet lookup;
				lookup << sv_check << word << (dictionary.count(word) == 1);

				socket.send(lookup, client_ip, client_port);

				break;
			}
			case cl_dump:
			{
				sf::Int16 dump_n;
				sf::Int8 chr;
				packet >> dump_n >> chr;

				if (game->check_dump(id, dump_n))
				{
					sf::Packet dump;

					string letters = game->dump(id, dump_n, chr);

					cout << endl << game->get_player_name(id) << " dumped " << chr
					     << "\n\tand received " << letters;
					cout.flush();

					dump << sv_dump << dump_n << letters;

					socket.send(dump, client_ip, client_port);
				}

				break;
			}
			case cl_peel:
			{
				sf::Int16 client_peel;
				packet >> client_peel;

				// make sure peel number is correct
				if (game->check_peel(client_peel))
				{
					// store first peeler id
					if (peeler.size() == 0)
						peeler = id;
					else
					{
						cout << "\n" << game->get_player_name(id) << " peeled too. Waiting on some players...";
						cout.flush();
					}
				}
				else
				{
					cout << "\nPeel out of order: got " << (int)client_peel << ", expected " << (int)game->get_peel();
					cout.flush();
				}

				break;
			}
			case cl_ack:
			{
				sf::Int16 ack_num;
				packet >> ack_num;

				Player& player = game->get_players().at(id);

				// if we were waiting for this ACK
				if (player.acknowledged(ack_num))
				{
					// if they have more pending, send the first
					if (player.has_pending())
					{
						sf::Packet pending(player.get_pending());
						socket.send(pending, player.get_ip(), client_port);
					}
					else // check if game can proceed
						game->check_waiting();
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

		// send pending packets
		for (auto& pair : game->get_players())
		{
			float elapsed = timer.getElapsedTime().asSeconds();
			timer.restart();

			if (pair.second.has_pending())
			{
				pair.second.step(elapsed);

				if (pair.second.get_timeout() > 5.f)
					game->remove_player(pair.first);
				else if (pair.second.poll > 0.5f)
				{
					pair.second.poll -= 0.5f;
					sf::Packet pending(pair.second.get_pending());
					socket.send(pending, pair.second.get_ip(), client_port);
				}
			}
		}

		// peel
		if (game->can_peel())
		{
			cout << endl;
			if (peeler.size() == 0)
				cout << "Split!";
			else
				cout << game->get_player_name(peeler) << ": Peel!";

			// if victory
			if (game->peel())
			{
				cout << "\n" << game->get_player_name(id) << " has won the game!";
				cout.flush();

				// send victory notification
				for (const auto& pair : game->get_players())
				{
					sf::Packet win;
					win << sv_done << sf::Uint8(1) << id;
					socket.send(win, pair.second.get_ip(), client_port);
					// TODO ack, also this is totally broken
				}
			}
			else
			{
				sf::Int16 remaining = game->get_remaining();

				for (auto& pair : game->get_players())
				{
					std::string letters = pair.second.get_peel();

					cout << "\nSending " << pair.second.get_name() << " " << letters;

					sf::Packet peel;
					peel << sv_peel << sf::Int16(game->get_peel() - 1) << remaining << pair.first << letters;

					if (!pair.second.has_pending())
						socket.send(peel, pair.second.get_ip(), client_port);

					pair.second.add_pending(peel);
					game->wait();
				}
			}

			cout.flush();
		}
	}

	return 0;
}

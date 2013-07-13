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
	po::options_description desc("Bananagrams multiplayer dedicated server");
	desc.add_options()
		("help", "show options")
		("dict", po::value<string>(), "dictionary file")
		("port", po::value<unsigned short>()->default_value(default_server_port), "TCP/UDP listening port")
		("bunch", po::value<string>()->default_value("1"), "bunch multiplier (0.5 or a positive integer)")
		("limit", po::value<unsigned int>(), "player limit")
	;

	// TODO usage string
	// TODO print help on unrecognized options
	po::variables_map opts;
	po::store(po::parse_command_line(argc, argv, desc), opts);
	po::notify(opts);

	if (opts.count("help"))
	{
		cerr << desc << endl;
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

	game = new Game(socket, b_num, b_den, max_players);

	bool try_start_game = false;

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

		// TODO send player info packets
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

				Player player(client_ip);
				packet >> player;

				if (game->add_player(id, player))
				{
					sf::Packet accept;
					accept << sv_connect << sf::Uint8(players.size() - 1);
					socket.send(accept, player.get_ip(), client_port);
				}
				break;
			}
			case cl_disconnect:
			{
				if (game->remove_player(id))
				{
					// TODO notify other players of disconnect/victory
				}
				break;
			}
			case cl_ready:
			{
				bool ready;
				packet >> ready;

				game->set_ready(id, ready);

				// TODO notify players of ready status
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
				sf::Int8 chr;
				packet >> chr;

				cout << endl << players[id].get_name() << " dumped " << chr;
				cout.flush();

				sf::Packet dump;
				dump << sv_dump << game->dump(chr);

				socket.send(dump, client_ip, client_port);

				break;
			}
			case cl_peel:
			{
				sf::Int16 client_peel;
				packet >> client_peel;

				if (game->peel(id, client_peel))
				{
					if (game->is_over())
					{
						string winner {game->winner()};

						// send victory notification
						for (const auto& pair : game->get_players())
						{
							sf::Packet win;
							win << sv_done << sf::Uint8(1) << winner;
							socket.send(win, pair.second.get_ip(), client_port);
						}
					}
					else
					{
						// send each player a new letter
						for (const auto& pair : game->get_players())
						{
							string letter = game->next_letter();

							cout << "\n" << "Sending " << pair.second.get_name() << " " << letter;
							cout.flush();

							sf::Packet peel;
							peel << sv_peel << sf::Int16(game->current_peel()) << remaining << id << letter;
							socket.send(peel, pair.second.get_ip(), client_port);
						}
					}
				}

				break;
			}
			default:
				cout << "\nUnrecognized packet type: " << (int)type;
				cout.flush();
		}

		if (try_start_game)
		{
			cout << "\nTrying to start game...";

			if (players.size() >= 2)
			{
				bool all_ready = true;
				for (const auto& pair : players)
				{
					if (!pair.second.ready)
					{
						all_ready = false;
						break;
					}
				}

				if (all_ready)
				{
					playing = true;

					// TODO determine number of letters based on bunch size/player number
					sf::Int16 remaining = bunch.size() - 21 * players.size();

					for (const auto& pair : players)
					{
						string letters;
						for (unsigned int i = 0; i < 21; i++)
						{
							letters.append(1, bunch.back());
							bunch.pop_back();
						}

						cout << "\n" << "Sending " << pair.second.get_name() << " " << letters;
						cout.flush();

						sf::Packet peel;
						peel << sv_peel << sf::Int16(peel_n) << remaining << pair.first << letters;
						socket.send(peel, pair.second.get_ip(), client_port);
					}
				}
				else
					cout << "\n\tNot all players are ready";
			}
			else
				cout << "\n\tNot enough players";

			cout.flush();

			try_start_game = false;
		}
	}

	return 0;
}

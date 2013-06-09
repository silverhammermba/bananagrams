#include "server.hpp"

namespace po = boost::program_options;
using std::cout;
using std::cerr;
using std::endl;
using std::string;

sf::UdpSocket socket;
unsigned short client_port;

std::map<string, Player> players;

void shutdown(int s)
{
	(void)s; // intentionally unused

	cout << "\nServer shutting down...";
	for (const auto& pair: players)
	{
		sf::Packet sorry;
		sorry << sf::Uint8(2);
		socket.send(sorry, pair.second.get_ip(), client_port);
	}
	cout << endl;
	exit(0);
}

int main(int argc, char* argv[])
{
	struct sigaction action;
	action.sa_handler = shutdown;
	sigemptyset(&action.sa_mask);
	action.sa_flags = 0;

	sigaction(SIGINT, &action, nullptr);
	sigaction(SIGTERM, &action, nullptr);
	sigaction(SIGKILL, &action, nullptr);

	po::options_description desc("Bananagrams multiplayer dedicated server");
	desc.add_options()
		("help", "show options")
		("dict", po::value<string>(), "dictionary file")
		("port", po::value<unsigned short>()->default_value(default_server_port), "TCP/UDP listening port")
		("bunch", po::value<string>()->default_value("1"), "bunch multiplier (0.5 or a positive integer)")
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

	// check bunch multiplier option
	std::stringstream multi_s;
	multi_s << opts["bunch"].as<string>();
	unsigned int b_num {1};
	unsigned int b_den {1};
	if (multi_s.str() == "0.5")
		b_den = 2;
	else
		multi_s >> b_num;

	// check dictionary option
	if (!opts.count("dict"))
	{
		cerr << "No dictionary file specified!\n";
		return 1;
	}

	string dict = opts["dict"].as<string>();
	std::map<string, string> dictionary;

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

	// LET'S GO!!!
	std::list<char> bunch;
	for (char ch = 'A'; ch <= 'Z'; ++ch)
		for (unsigned int i = 0; i < ((letter_count[ch - 'A'] * b_num) / b_den); ++i)
			random_insert(bunch, ch);

	// TODO catch failure
	socket.bind(server_port);

	sf::Uint8 peel_n = 0;
	bool playing = false;

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

		switch(type)
		{
			case 0: // player join
			{
				sf::Uint8 version;
				packet >> version;
				if (version != protocol_version)
				{
					sf::Packet sorry;
					sorry << sf::Uint8(1) << sf::Uint8(0);
					socket.send(sorry, client_ip, client_port);

					cout << "\nclient failed to join"
						    "\n\tNeed protocol version " << (int)protocol_version << ", got " << (int)version;
					cout.flush();
					break;
				}

				if (playing)
				{
					sf::Packet sorry;
					sorry << sf::Uint8(1) << sf::Uint8(3);
					socket.send(sorry, client_ip, client_port);

					cout << "\nclient failed to join"
						    "\n\tGame already started";
					cout.flush();
					break;
				}

				if (players.count(id) == 0)
				{
					Player player(client_ip);
					packet >> player;
					players[id] = player;

					cout << "\n" << player.get_name() << " has joined the game";
					cout.flush();

					sf::Packet accept;
					accept << sf::Uint8(0) << sf::Uint8(players.size() - 1);
					socket.send(accept, player.get_ip(), client_port);

					playing = true;

					sf::Int16 remaining = bunch.size() - 21 * players.size();

					// TODO for now, start the game immediately
					for (const auto& pair : players)
					{
						string letters;
						for (unsigned int i = 0; i < 21; i++)
						{
							letters.append(1, bunch.back());
							bunch.pop_back();
						}

						cout << "\n" << "Sending " << player.get_name() << " " << letters;
						cout.flush();

						sf::Packet peel;
						peel << sf::Uint8(5) << sf::Uint8(peel_n) << remaining << pair.first << letters;
						socket.send(peel, pair.second.get_ip(), client_port);
					}
				}
				break;
			}
			case 1: // player disconnect
			{
				if (players.count(id) > 0)
				{
					cout << "\n" << players[id].get_name() << " has left the game";
					cout.flush();
					players.erase(id);
				}
				break;
			}
			case 4:
			{
				string word;
				packet >> word;

				sf::Packet lookup;
				lookup << sf::Uint8(4) << word;

				lookup << (dictionary.count(word) == 1);

				socket.send(lookup, client_ip, client_port);

				break;
			}
			case 5:
			{
				sf::Int8 chr;
				packet >> chr;

				cerr << endl << players[id].get_name() << " dumped " << chr;
				cerr.flush();

				string letters;

				sf::Packet dump;
				dump << sf::Uint8(6);

				if (bunch.size() >= 3)
				{
					// take three
					for (unsigned int i = 0; i < 3; i++)
					{
						letters.append(1, bunch.back());
						bunch.pop_back();
					}

					random_insert(bunch, (char)chr);
				}
				else
				{
					letters.append(1, (char)chr);
					cerr << "\n\tNot enough letters left for dump";
					cerr.flush();
				}

				dump << letters;

				socket.send(dump, client_ip, client_port);

				break;
			}
			case 6: // finished peel
			{
				if (players.count(id) > 0)
				{
					sf::Uint8 client_peel;
					packet >> client_peel;

					if (client_peel == peel_n + 1)
					{
						sf::Int16 remaining = bunch.size() - players.size();

						// if there aren't enough letters left
						if (remaining < 0)
						{
							// send victory notification
							for (const auto& pair : players)
							{
								sf::Packet win;
								win << sf::Uint8(7) << sf::Uint8(1) << id;
								socket.send(win, pair.second.get_ip(), client_port);
							}

							cout << "\n" << players[id].get_name() << " has won the game!";
							cout.flush();
							shutdown(0);
						}

						++peel_n;
						cout << "\n" << players[id].get_name() << " peeled (" << (int)peel_n << ")";
						cout.flush();

						// send each player a new letter
						for (const auto& pair : players)
						{
							string letter;
							letter.append(1, bunch.back());
							bunch.pop_back();

							cout << "\n" << "Sending " << pair.second.get_name() << " " << letter;
							cout.flush();

							sf::Packet peel;
							peel << sf::Uint8(5) << sf::Uint8(peel_n) << remaining << id << letter;
							socket.send(peel, pair.second.get_ip(), client_port);
						}
					}
					else
					{
						cout << "\nPeel out of order: got " << (int)client_peel << ", expected " << (int)(peel_n + 1);
						cout.flush();
					}
				}

				break;
			}
			default:
				cout << "\nUnrecognized packet type: " << (int)type;
				cout.flush();
		}
	}

	return 0;
}

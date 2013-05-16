#include "server.hpp"

namespace po = boost::program_options;
using std::cout;
using std::cerr;
using std::endl;
using std::string;

std::map<string, Player> players;

void shutdown(int s)
{
	cout << "\nServer shutting down...";
	// TODO send disconnects to players
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

	sf::UdpSocket socket;
	// TODO catch failure
	socket.bind(server_port);

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
		packet >> type;

		string id;

		switch(type)
		{
			case 0: // player join
			{
				sf::Uint8 version;
				packet >> version;
				if (version != protocol_version)
				{
					// TODO send client rejection
					cout << "\nclient failed to join"
						    "\n\tNeed protocol version " << (int)protocol_version << ", got " << (int)version;
					cout.flush();
					break;
				}
				packet >> id;

				if (players.count(id) == 0)
				{
					Player player;
					packet >> player;
					players[id] = player;
					// TODO not getting name properly
					cout << "\n" << player.get_name() << " has joined the game";
					cout.flush();
				}
				break;
			}
			case 1: // player disconnect
			{
				packet >> id;

				if (players.count(id) > 0)
				{
					cout << "\n" << players[id].get_name() << " has left the game";
					cout.flush();
					players.erase(id);
				}
				break;
			}
			default:
				cout << "\nUnrecognized packet type: " << type;
				cout.flush();
		}
	}
	// TODO catch interrupt and inform clients

	return 0;
}

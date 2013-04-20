#include "server.hpp"

namespace po = boost::program_options;
using std::cout;
using std::cerr;
using std::endl;
using std::string;

int main(int argc, char* argv[])
{
	po::options_description desc("Bananagrams multiplayer dedicated server");
	desc.add_options()
		("help", "show options")
		("dict", po::value<string>(), "dictionary file")
		("port", po::value<unsigned int>()->default_value(default_port), "TCP/UDP listening port")
		("bunch", po::value<string>()->default_value("1"), "bunch multiplier (0.5, inf, or a positive integer)")
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
	unsigned int server_port {opts["port"].as<unsigned int>()};
	if (server_port == 0 || server_port > 65535)
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
	{
		b_den = 2;
	}
	else if (multi_s.str() == "inf")
	{
		// TODO
	}
	else
	{
		multi_s >> b_num;
	}

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

	// LET'S GO!!!
	std::list<char> bunch;
	for (char ch = 'A'; ch <= 'Z'; ++ch)
		for (unsigned int i = 0; i < ((letter_count[ch - 'A'] * b_num) / b_den); ++i)
			random_insert(bunch, ch);

	sf::UdpSocket socket;
	socket.bind(server_port);

	cout << "\nWaiting for player connections...";
	while (true)
	{
		sf::Packet packet;
		sf::IpAddress client_ip;
		unsigned short client_port;
		socket.receive(packet, client_ip, client_port);
		cout << "\nReceived packet from " << client_ip << ":" << client_port;
		sf::Uint8 type;
		packet >> type;
		switch(type)
		{
			case 0:
			{
				std::string name;
				packet >> name;
				cout << "\n" << name << " is attempting to join";
				break;
			}
			default:
				cout << "\nUnrecognized packet type: " << type;
		}
	}

	return 0;
}

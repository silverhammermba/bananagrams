#include "server.hpp"

namespace po = boost::program_options;
using std::cout;
using std::cerr;
using std::endl;

int main(int argc, char* argv[])
{
	po::options_description desc("Bananagrams multiplayer dedicated server");
	desc.add_options()
		("help", "show options")
		("dict", po::value<std::string>(), "dictionary file")
		("port", po::value<unsigned int>()->default_value(57198), "TCP/UDP listening port")
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

	unsigned int port {opts["port"].as<unsigned int>()};
	if (port == 0 || port > 65535)
	{
		cerr << "Invalid listening port: " << port << "!\n";
		return 1;
	}

	if (!opts.count("dict"))
	{
		cerr << "No dictionary file specified!\n";
		return 1;
	}

	std::string dict = opts["dict"].as<std::string>();
	std::map<std::string, std::string> dictionary;

	cout << "Loading dictionary... ";
	cout.flush();
	std::ifstream words(dict);
	if (!words.is_open())
	{
		std::cerr << "\nFailed to open " << dict << "!\n";
		return 1;
	}

	// parse dictionary
	std::string line;
	while (std::getline(words, line))
	{
		auto pos = line.find_first_of(' ');
		if (pos == std::string::npos)
			dictionary[line] = "";
		else
			dictionary[line.substr(0, pos)] = line.substr(pos + 1, std::string::npos);
	}
	words.close();
	cout << dictionary.size() << " words found\n";

	while (true)
	{
	}

	return 0;
}

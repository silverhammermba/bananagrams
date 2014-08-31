#include <csignal>
#include <string>

#include <boost/program_options.hpp>

#include "server.hpp"

namespace po = boost::program_options;
using std::cout;
using std::cerr;
using std::endl;
using std::string;

Server* server;

// callback for interrupt signal
void shutdown(int s)
{
	(void)s; // intentionally unused

	server->shutdown();
}

int main(int argc, char* argv[])
{
#ifndef __MINGW32__
	// set up shutdown callback for interrupts
	struct sigaction action;
	action.sa_handler = shutdown;
	sigemptyset(&action.sa_mask);
	action.sa_flags = 0;

	sigaction(SIGINT , &action, nullptr);
	sigaction(SIGTERM, &action, nullptr);
	sigaction(SIGKILL, &action, nullptr);
#endif

	// command line arguments
	po::options_description desc("Bananagrams dedicated server options");
	desc.add_options()
		("help",                                                                   "show options")
		("dict",  po::value<string>()->required(),                                 "dictionary file")
		("port",  po::value<unsigned short>()->default_value(default_server_port), "TCP/UDP listening port")
		("bunch", po::value<string>()->default_value("1"),                         "bunch multiplier (0.5 or a positive integer)")
		("limit", po::value<unsigned int>(),                                       "player limit")
	;

	po::variables_map opts;

	try
	{
		po::store(po::parse_command_line(argc, argv, desc), opts);

		if (opts.count("help"))
		{
			cerr << desc << endl;
			return 1;
		}

		po::notify(opts);
	}
	catch (po::error& e)
	{
		cerr << "Error: " << e.what() << endl << endl << desc << endl;
		return 1;
	}

	// check port option
	unsigned short server_port {opts["port"].as<unsigned short>()};

	// game options
	int b_num;
	unsigned int b_den;
	unsigned int max_players;

	b_num = 1;
	b_den = 1;

	// check bunch multiplier option
	if (opts.count("bunch"))
	{
		string multi_s;
		multi_s = opts["bunch"].as<string>();

		if (multi_s == "0.5")
			b_den = 2;
		else
		{
			size_t unconverted;
			b_num = std::stoi(multi_s, &unconverted);

			if (unconverted < multi_s.length() || b_num < 1)
			{
				cerr << "Error: invalid bunch multiplier " << multi_s << endl;
				return 1;
			}
		}
	}

	max_players = (8 * b_num) / b_den;

	if (opts.count("limit"))
	{
		auto limit = opts["limit"].as<unsigned int>();

		if (limit > max_players)
			cout << "Max player limit is " << max_players << " for this bunch size\n";
		else if (limit < 2)
			cout << "Min player limit is 2\n";
		else
			max_players = limit;
	}

	string dict = opts["dict"].as<string>();

	bool done = false;
	while (!done)
	{
		server = new Server(server_port, dict, b_num, b_den, max_players);

		switch(server->block())
		{
			case Server::Status::RUNNING:
				cerr << "\nError: server thread aborted while running!\n";
				return 1;
			case Server::Status::ABORTED:
				done = true;
				break;
			case Server::Status::DONE:
				cout << "\nRestarting server...";
				cout.flush();
				break;
		}

		delete server;
	}

	cout << endl;
	return 0;
}

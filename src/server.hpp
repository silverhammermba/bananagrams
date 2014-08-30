class Server
{
	// parameters for creation
	std::string dict_filename;
	uint8_t bunch_num;
	uint8_t bunch_den;
	unsigned int player_limit;

	// internal structures
	Bunch* bunch;
	std::map<std::string, Player> players;

	// state vars
	sf::Int16 peel_number {0};
	bool playing {false};
	bool ready_to_peel {false}; // game is ready for next peel
	bool waiting {false}; // waiting for clients to acknowledge critical server packets before next peel
	bool ready_to_finish {false}; // true if game just finished
	bool finished {false};

	std::thread thread;
public:
	Server(unsigned short port, const std::string& _dict_filename, uint8_t _num, uint8_t _den, unsigned int _max_players);
	Server(std::ifstream& save_file);
	~Server();

	void save(const std::string& filename);
};

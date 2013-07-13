class Game
{
	sf::UdpSocket& socket;
	unsigned short client_port;
	unsigned int player_limit;
	std::list<char> bunch;
	std::map<std::string, Player> players;
	sf::Int16 peel_number {0};
	bool playing {false};
	std::string winner {""};

	void try_to_start();
public:
	Game(unsigned int _bunch_num, unsigned int _bunch_den, unsigned int _player_limit);

	inline bool is_full() const
	{
		return players.size() == player_limit;
	}

	inline bool in_progress() const
	{
		return playing;
	}

	inline bool is_over() const
	{
		return winner != "";
	}

	inline const sf::Int16& current_peel() const
	{
		return peel_number;
	}

	std::string dump(char chr);

	bool add_player(const std::string& id, Player& player);
	bool remove_player(const std::string& id);
	void set_ready(const std::string& id, bool ready);
	bool peel(const std::string& id, sf::Int16 number);
	std::string next_letter();
};

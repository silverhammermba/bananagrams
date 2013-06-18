class Game
{
	sf::Socket& socket;
	unsigned int player_limit;
	std::list<char> bunch;
	std::map<std::string, Player> players;
	sf::Int16 peel_number {0};
	bool playing {false};
	std::string winner {""};
public:
	Game(sf::Socket& _socket, unsigned int _bunch_num, unsigned int _bunch_den, unsigned int _player_limit)
	~Game();

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

	bool add_player(const std::string& id, Player& player);
	bool remove_player(const std::string& id);
	void set_ready(const std::string& id, bool ready);
	bool peel(const string& id, sf::Int16 number);
	string next_letter();
};

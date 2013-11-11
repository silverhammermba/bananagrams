class Game
{
	unsigned int player_limit;
	std::list<char> bunch;
	std::map<std::string, Player> players;
	sf::Int16 peel_number {0};
	bool playing {false};
	bool must_start {false};

	void try_to_start();
public:
	Game(unsigned int _bunch_num, unsigned int _bunch_den, unsigned int _player_limit);

	inline bool has_player(const std::string& id) const
	{
		return players.count(id) != 0;
	}

	inline const std::map<std::string, Player>& get_players() const
	{
		return players;
	}

	inline const std::string& get_player_name(const std::string& id) const
	{
		return players.at(id).get_name();
	}

	inline unsigned int get_remaining() const
	{
		return bunch.size();
	}

	inline bool is_full() const
	{
		return players.size() == player_limit;
	}

	inline bool in_progress() const
	{
		return playing;
	}

	inline bool should_start() const
	{
		return must_start;
	}

	inline const sf::Int16& current_peel() const
	{
		return peel_number;
	}

	inline bool check_dump(const std::string& id, const sf::Int16& dump_n) const
	{
		return (dump_n == players.at(id).get_dump() - 1) || (dump_n == players.at(id).get_dump());
	}

	inline bool check_peel(const sf::Int16& number) const
	{
		return number == peel_number + 1;
	}

	inline const sf::Int16& get_peel() const
	{
		return peel_number;
	}

	std::string dump(const std::string& id, const sf::Int16& dump_n, char chr);

	void add_player(const std::string& id, const sf::IpAddress& ip, const std::string& name);
	void remove_player(const std::string& id);
	void set_ready(const std::string& id, bool ready);
	bool peel();
	void start();
};

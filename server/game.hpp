class Game
{
	unsigned int player_limit;
	std::list<char> bunch;
	std::map<std::string, Player> players;
	sf::Int16 peel_number {0};
	bool playing {false};
	bool ready_to_peel {false}; // game is ready for next peel
	bool waiting {false}; // waiting for clients to acknowledge critical server packets before next peel
	bool ready_to_finish {false}; // true if game just finished
	bool finished {false};

	void try_to_start();
public:
	std::string winner;

	Game(unsigned int _bunch_num, unsigned int _bunch_den, unsigned int _player_limit);

	inline bool has_player(const std::string& id) const
	{
		return players.count(id) != 0;
	}

	inline std::map<std::string, Player>& get_players()
	{
		return players;
	}

	inline const std::string& get_player_name(const std::string& id) const
	{
		return players.at(id).get_name();
	}

	inline sf::Int16 get_remaining() const
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

	inline bool can_peel() const
	{
		return ready_to_peel && !waiting;
	}

	inline bool is_finished() const
	{
		return finished;
	}

	inline bool is_ready_to_finish() const
	{
		return ready_to_finish;
	}

	inline void finish()
	{
		ready_to_finish = false;
	}

	inline bool can_restart() const
	{
		return finished && !waiting;
	}

	inline void wait()
	{
		waiting = true;
	}

	void check_waiting();

	inline bool check_dump(const std::string& id, const sf::Int16& dump_n) const
	{
		return (dump_n == players.at(id).get_dump() - 1) || (dump_n == players.at(id).get_dump());
	}

	inline bool check_peel(const sf::Int16& number)
	{
		if (number == peel_number)
			ready_to_peel = true;
		return ready_to_peel;
	}

	inline const sf::Int16& get_peel() const
	{
		return peel_number;
	}

	std::string dump(const std::string& id, const sf::Int16& dump_n, char chr);

	Player& add_player(const std::string& id, const sf::IpAddress& ip, unsigned short port, const std::string& name);
	void remove_player(const std::string& id);
	void set_ready(const std::string& id, bool ready);
	bool peel();
	void start();
	void got_ack(const std::string& id, const sf::Int16& ack_num) const;
};

// TODO keep track of letters so they can be readded to bunch upon disconnect

class Player
{
	sf::IpAddress ip;
	std::string name;
	std::string dump_letters;
	sf::Int16 dump_n {0};
	std::list<char> hand;

	float timeout;
	sf::Int16 ack_count {0};
	std::list<sf::Packet*> pending;
public:
	bool ready {false};
	float poll;

	explicit Player() {}; // XXX to appease the players map
	Player(const sf::IpAddress& _ip, const std::string& _name);
	~Player();

	inline const sf::IpAddress& get_ip() const
	{
		return ip;
	}

	inline const std::string& get_name() const
	{
		return name;
	}

	inline const std::string& last_dump() const
	{
		return dump_letters;
	}

	inline const sf::Int16& get_dump() const
	{
		return dump_n;
	}

	inline std::list<char>& get_hand()
	{
		return hand;
	}

	inline float get_timeout() const
	{
		return timeout;
	}

	inline float get_poll() const
	{
		return poll;
	}

	std::string get_hand_str() const;

	inline char last_peel() const
	{
		return hand.back();
	}

	void give_dump(const std::string& letters);
	void give_peel(const std::string& letters);

	void give_split(const std::string& letters);

	void add_pending(const sf::Packet& packet)
	{
		// if this the first packet, reset timers
		if (pending.size() == 0)
		{
			timeout = 0;
			poll = 0;
		}
		pending.push_back(new sf::Packet(packet));
	}

	bool has_pending() const
	{
		return pending.size() > 0;
	}

	const sf::Packet& get_pending() const
	{
		return *pending.front();
	}

	void step(float elapsed)
	{
		timeout += elapsed;
		poll += elapsed;
	}

	bool acknowledged(const sf::Int16& ack_num)
	{
		if (pending.size() == 0)
			return false;

		if (ack_num != ack_count)
			return false;

		++ack_count;
		delete pending.front();
		pending.pop_front();

		return true;
	}

	void reset_ack()
	{
		ack_count = 0;
		// pending should already be empty
	}
};

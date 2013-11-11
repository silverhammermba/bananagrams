// TODO keep track of letters so they can be readded to bunch upon disconnect

class Player
{
	sf::IpAddress ip;
	std::string name;
	std::string dump_letters;
	sf::Int16 dump_n {0};
	sf::Packet* pending {nullptr};
	std::list<char> hand;
public:
	bool ready {false};

	explicit Player() {}; // XXX to appease the players map
	Player(const sf::IpAddress& _ip, const std::string& _name);

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

	std::string get_hand_str() const;

	inline char last_peel() const
	{
		return hand.back();
	}

	void give_dump(const std::string& letters);
	inline void give_peel(char letter)
	{
		hand.push_back(letter);
		// TODO ACK peel so that this doesn't get overwritten
	}

	void give_split(const std::string& letters);
};

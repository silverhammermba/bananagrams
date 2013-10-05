// TODO keep track of letters so they can be readded to bunch upon disconnect

class Player
{
	sf::IpAddress ip;
	std::string name;
	std::vector<std::string> dumps;
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

	inline const std::vector<std::string>& get_dumps() const
	{
		return dumps;
	}

	void add_dump(const std::string& letters);
};

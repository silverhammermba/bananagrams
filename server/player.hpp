// TODO keep track of letters so they can be readded to bunch upon disconnect

class Player
{
	sf::IpAddress ip;
	std::string name;
public:
	bool ready {false};

	explicit Player() {}; // XXX to appease the players map
	Player(const sf::IpAddress& _ip);

	inline const sf::IpAddress& get_ip() const
	{
		return ip;
	}

	inline const std::string& get_name() const
	{
		return name;
	}

	friend sf::Packet& operator >>(sf::Packet& packet, Player& player);
};

sf::Packet& operator >>(sf::Packet& packet, Player& player);

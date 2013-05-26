class Player
{
	sf::IpAddress ip;
	std::string name;
	unsigned int peel;
public:
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

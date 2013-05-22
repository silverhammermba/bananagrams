class Player
{
	std::string name;
	unsigned int peel;
public:
	Player();

	inline const std::string& get_name() const
	{
		return name;
	}

	friend sf::Packet& operator >>(sf::Packet& packet, Player& player);
};

sf::Packet& operator >>(sf::Packet& packet, Player& player);

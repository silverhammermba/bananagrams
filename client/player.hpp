class Player
{
	std::string name;
public:
	sf::Uint16 letters;

	Player(const std::string& n)
		: name {n}
	{}

	inline const std::string& get_name() const
	{
		return name;
	}

	friend sf::Packet& operator >>(sf::Packet& packet, Player& player);
};

sf::Packet& operator >>(sf::Packet& packet, Player& player);

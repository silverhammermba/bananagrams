class Player
{
	std::string name;
public:
	bool ready;

	explicit Player() {}; // XXX to appease the players map

	Player(const std::string& _name)
		: name {_name}, ready {false}
	{
	}

	inline const std::string& get_name() const
	{
		return name;
	}
};

class Player
{
	std::string name;
	unsigned int peel;
public:
	Player();

	inline void set_name(const std::string& nm)
	{
		name = nm;
	}

	inline const std::string& get_name() const
	{
		return name;
	}
};

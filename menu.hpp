class Menu : public InputReader
{
	sf::Text title;
	std::vector<sf::Text> entries;
public:
	Menu(const std::string& ttl, const std::vector<std::string>& ents);

	void draw_on(sf::RenderWindow& window) const;

	inline void enable()
	{
		finished = false;
	}

	virtual bool process_event(sf::Event& event);
};

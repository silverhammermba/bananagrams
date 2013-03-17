class Menu : public InputReader
{
	const sf::View& view;
	sf::Text title;
	std::vector<sf::Text> entries;
	unsigned int highlighted;
	sf::RectangleShape background;

	void highlight(unsigned int i);
	void select(unsigned int i);
public:
	Menu(const sf::View& vw, const std::string& ttl, const std::vector<std::string>& ents, float size);

	void draw_on(sf::RenderWindow& window) const;

	inline void enable()
	{
		finished = false;
	}

	virtual bool process_event(sf::Event& event);
};

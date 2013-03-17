class Submenu
{
	const sf::View& view;
	Submenu* const parent;
	sf::Text title;
	std::vector<sf::Text> entries;
	std::vector<Submenu*> submenus;
	unsigned int highlighted;
	sf::RectangleShape background;
	float size;
public:
	Submenu(const sf::View& vw, Submenu* p, const std::string& ttl, float sz);

	inline Submenu* get_parent() const
	{
		return parent;
	}

	inline unsigned int get_size() const
	{
		return entries.size();
	}

	void add_entry(const std::string& entry, Submenu* sub);

	void highlight(unsigned int i);
	inline void highlight_prev()
	{
		highlight(highlighted == 0 ? entries.size() - 1 : highlighted - 1);
	}
	inline void highlight_next()
	{
		highlight(highlighted == entries.size() - 1 ? 0 : highlighted + 1);
	}
	void highlight_coords(float x, float y);

	inline Submenu* select()
	{
		return submenus[highlighted];
	}

	void draw_on(sf::RenderWindow& window) const;
};

class Menu : public InputReader
{
	Submenu* submenu;
public:
	Menu(Submenu& root);

	void draw_on(sf::RenderWindow& window) const;

	inline void enable()
	{
		finished = false;
	}

	virtual bool process_event(sf::Event& event);
};

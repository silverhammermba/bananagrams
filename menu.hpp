class Entry : public InputReader
{
	sf::Text text;
public:
	Entry(const std::string& txt);

	virtual float get_width() const;
	virtual float get_height() const;
	virtual sf::FloatRect bounds() const;
	virtual void set_menu_pos(float top, float width);

	virtual void highlight();
	virtual void lowlight();

	virtual void draw_on(sf::RenderWindow& window) const;

	virtual void select() = 0;
};

class Menu : public InputReader
{
	const sf::View& view;
	Menu* parent;
	sf::Text title;
	std::list<Entry*> entries;
	std::list<Entry*>::iterator highlighted;
	sf::RectangleShape background;
public:
	Menu(const sf::View& vw, Menu* p, const std::string& ttl);

	inline Menu* get_parent() const
	{
		return parent;
	}

	inline unsigned int get_size() const
	{
		return entries.size();
	}

	void add_entry(std::list<Entry*>::iterator it, Entry* entry);
	inline void append_entry(Entry* entry)
	{
		add_entry(entries.end(), entry);
	}
	void remove_entry(Entry* entry);

	void highlight(std::list<Entry*>::iterator it);
	void highlight_prev();
	void highlight_next();
	void highlight_coords(float x, float y);

	void draw_on(sf::RenderWindow& window) const;
	virtual bool process_event(sf::Event& event);
};

class MenuWrapper : public InputReader
{
public:
	Menu** menu;
	MenuWrapper(Menu** current) { menu = current; };

	inline void enable()
	{
		finished = false;
	}

	inline virtual bool process_event(sf::Event& event)
	{
		bool go = (*menu)->process_event(event);
		finished = (*menu)->is_finished();
		return go;
	}
};

class MenuEntry : public Entry
{
	Menu** root;
public:
	Menu* submenu;
	MenuEntry(std::string txt, Menu** rt, Menu* sub = nullptr);
	virtual void select();
	virtual bool process_event(sf::Event& event);
};

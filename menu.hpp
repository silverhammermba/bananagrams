class Entry : public InputReader
{
protected:
	sf::Text text;
public:
	Entry(const std::string& txt);

	virtual float get_width() const;
	virtual float get_height() const;
	virtual sf::FloatRect bounds() const;
	virtual void set_menu_pos(float center, float width, float top);

	virtual void highlight();
	virtual void lowlight();

	virtual void draw_on(sf::RenderWindow& window) const;

	virtual void select() = 0;
};

class MenuSystem;

class Menu : public InputReader
{
	const sf::View& view;
	MenuSystem& system;
	Menu* parent;
	sf::Text title;
	std::list<Entry*> entries;
	std::list<Entry*>::iterator highlighted;
	sf::RectangleShape background;
public:
	Menu(const sf::View& vw, MenuSystem& sys, Menu* p, const std::string& ttl);

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

class MenuSystem : public InputReader
{
	// TODO dangerous if not initialized
	Menu* menu_p;
public:
	MenuSystem() {}

	inline void open()
	{
		finished = false;
	}

	inline void close()
	{
		finished = true;
	}

	inline void set_menu(Menu& m)
	{
		menu_p = &m;
	}

	inline Menu& menu() const
	{
		return *menu_p;
	}

	virtual bool process_event(sf::Event& event);
};

// go to a submenu
class MenuEntry : public Entry
{
	MenuSystem& system;
public:
	Menu* submenu;
	MenuEntry(const std::string& txt, MenuSystem& sys, Menu* sub = nullptr);
	virtual void select();
};

// start a solitaire game
class SolitaireEntry : public Entry
{
public:
	SolitaireEntry(const std::string& txt);

	virtual void select();
};

// enter text
class TextEntry : public Entry
{
	sf::RectangleShape box;
	sf::Text input;
	std::string str;
	std::string default_display;
	std::string default_str;
	bool selected;
public:
	TextEntry(const std::string& txt, const std::string& def_display, const std::string& def);

	inline const std::string& get_string()
	{
		return str;
	}

	virtual float get_width();
	virtual void set_menu_pos(float center, float width, float top);
	virtual void highlight();
	virtual void lowlight();
	virtual void select();
	virtual bool process_event(sf::Event& event);
	virtual void draw_on(sf::RenderWindow& window) const;
};

class MultiEntry : public Entry
{
	unsigned int choice;
	std::vector<std::string> choices;
public:
	MultiEntry(const std::string& txt, const std::vector<std::string>& ch);

	inline virtual void select() {} // nothing to do
	virtual bool process_event(sf::Event& event);
};

class QuitEntry : public Entry
{
	sf::RenderWindow& window;
public:
	QuitEntry(const std::string& txt, sf::RenderWindow& win);
	virtual void select();
};

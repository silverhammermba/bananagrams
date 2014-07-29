// ABC for entries in Menus
class Entry : public InputReader
{
protected:
	sf::Text text;
	float scale;
	bool pending;
public:
	Entry(const std::string& txt, float sz=1.f);

	// get minimum width occupied by this entry
	virtual float get_width() const;
	// TODO do we need this?
	// get minimum height occupied by this entry
	virtual float get_height() const;
	// get bounding FloatRect (for mouse selection)
	virtual sf::FloatRect bounds() const;
	// position entry given menu center, width, and entry top
	virtual void set_menu_pos(float center, float width, float top);
	virtual float get_scale() const;

	// called by Menu#update_entries
	virtual void update() {}

	// indicate that entry is current
	virtual void highlight();
	// remove that indication
	virtual void lowlight();

	virtual void draw_on(sf::RenderWindow& window) const;

	// TODO can I make an implementation for this?
	// triggered when return is pressed for this entry
	virtual void select()
	{
		pending = true;
	}

	inline bool is_pending()
	{
		bool p = pending;
		pending = false;
		return p;
	}
};

class MenuSystem;

// stores list of Entries and manages navigation
class Menu : public InputReader
{
	SoundManager& sound;
	MenuSystem& system; // system managing this menu
	Menu* parent; // nullptr for no parent
	sf::Text title;
	std::list<Entry*> entries;
	std::list<Entry*>::iterator highlighted;
	sf::RectangleShape background;
public:
	// create menu centered on vw, managed by sys, with parent p, and title ttl
	Menu(SoundManager& _sound, MenuSystem& sys, Menu* p, const std::string& ttl);

	inline Menu* get_parent() const
	{
		return parent;
	}

	void add_entry(std::list<Entry*>::iterator it, Entry* entry);
	inline void append_entry(Entry* entry)
	{
		add_entry(entries.end(), entry);
	}
	void remove_entry(Entry* entry);

	void update_entries()
	{
		for (auto& entry : entries)
			entry->update();
	}

	// call when Menu or View changes
	void update_position();

	// highlight entry
	void highlight(std::list<Entry*>::iterator it);
	void highlight_prev();
	void highlight_next();
	// use Entry#bounds to highlight
	void highlight_coords(float x, float y);

	void draw_on(sf::RenderWindow& window) const;

	// navigate menu and pass events to the highlighted entry
	virtual bool process_event(sf::Event& event);
};

// keeps track of current menu, delegates Events
class MenuSystem : public InputReader
{
	// TODO dangerous if not initialized
	Menu* menu_p;
public:
	MenuSystem() {}

	// use InputReader finished state to determine if menu should be dispayed
	inline void open()
	{
		finished = false;
	}

	inline void close()
	{
		finished = true;
	}

	// to switch to parent/child menus
	inline void set_menu(Menu& m)
	{
		menu_p = &m;
		menu_p->update_position();
	}

	inline Menu& menu() const
	{
		return *menu_p;
	}

	// forward events to current menu
	virtual bool process_event(sf::Event& event);
};

// change MenuSystem's Menu
class MenuEntry : public Entry
{
	MenuSystem& system;
public:
	Menu* submenu;
	MenuEntry(const std::string& txt, MenuSystem& sys, Menu* sub = nullptr);
	virtual void select();
};

// enter text
class TextEntry : public Entry
{
	sf::RectangleShape box;
	sf::Text input;
	std::string str; // internal input string
	std::string default_display; // what to display when input is empty
	std::string default_str; // what to set str to when input is empty
	// for setting input position
	float b_height, i_height, shift;
	float min_box_width;
	bool selected {false}; // if input is being handled

	void set_input_pos(); // update position of input Text
public:
	// create text entry at least mbw wide, storing def and displaying def_display for empty input
	TextEntry(const std::string& txt, float mbw, const std::string& def = "", const std::string& def_display = "");

	inline const std::string& get_string()
	{
		return str;
	}

	virtual float get_width() const;
	virtual sf::FloatRect bounds() const;
	// left align text, fill remaining space with box, center input text in box
	virtual void set_menu_pos(float center, float width, float top);
	virtual void highlight();
	virtual void lowlight();
	virtual void select();
	virtual bool process_event(sf::Event& event);
	virtual void draw_on(sf::RenderWindow& window) const;
};

// choose from multiple options
class MultiEntry : public Entry
{
	unsigned int choice; // index of selection
	std::vector<std::string> choices; // strings to display for choices
	sf::Text chooser; // display of choice
	// for layout
	float max_width {0};
	float set_width;
public:
	// create multiple choice entry with choices ch, defaulting to def
	MultiEntry(const std::string& txt, const std::vector<std::string>& ch, unsigned int def = 0);

	inline unsigned int get_choice() const
	{
		return choice;
	}
	virtual float get_width() const;
	virtual sf::FloatRect bounds() const;
	void update_choice();
	// left align text, center choice in remaining space
	virtual void set_menu_pos(float center, float width, float top);
	virtual void highlight();
	virtual void lowlight();
	// doesn't need a select
	virtual bool process_event(sf::Event& event);
	virtual void draw_on(sf::RenderWindow& window) const;
};

// TODO modularize better? how to get settings for starting game?
// TODO really need a nicer way of creating a game from the menu entry
class SingleplayerEntry : public Entry
{
	MenuSystem& system;
	TextEntry& dict_entry;
	MultiEntry& multiplier;

public:
	SingleplayerEntry(const std::string& txt, MenuSystem& sys, TextEntry& dict_entry, MultiEntry& multiplier);

	virtual void select();

	inline const std::string& get_dictionary() const
	{
		return dict_entry.get_string();
	}

	inline unsigned int get_multiplier() const
	{
		return multiplier.get_choice();
	}
};

// TODO modularize better? how to get settings for starting game?
class MultiplayerEntry : public Entry
{
	MenuSystem& system;
	TextEntry& server;
	TextEntry& name;

public:
	MultiplayerEntry(const std::string& txt, MenuSystem& sys, TextEntry& srv, TextEntry& nm);

	virtual void select();

	inline const std::string& get_server() const
	{
		return server.get_string();
	}

	inline const std::string& get_name() const
	{
		return name.get_string();
	}
};

class ControlEntry : public Entry
{
	Menu& control_menu;
	KeyControls& controls;
	std::string command;
	sf::Event::KeyEvent key;
	sf::RectangleShape box;
	sf::Text key_text;

	float b_shift, b_height, i_height, shift;
	float min_box_width;
	bool selected {false}; // if input is being handled
public:
	ControlEntry(Menu& cmenu, KeyControls& ctrls, const std::string& cmd, const sf::Event::KeyEvent& k);

	inline const std::string& get_command()
	{
		return command;
	}
	inline const sf::Event::KeyEvent& get_key()
	{
		return key;
	}

	void set_input_pos();

	// check if command was unbound
	void update();

	virtual float get_width() const;
	virtual sf::FloatRect bounds() const;
	// left align text, fill remaining space with box, center input text in box
	virtual void set_menu_pos(float center, float width, float top);
	virtual void highlight();
	virtual void lowlight();
	virtual void select();
	virtual bool process_event(sf::Event& event);
	virtual void draw_on(sf::RenderWindow& window) const;
};

// close the window
class QuitEntry : public Entry
{
	sf::RenderWindow& window;
public:
	QuitEntry(const std::string& txt, sf::RenderWindow& win);
	virtual void select();
};

#ifndef MENU_HPP
#define MENU_HPP

#include <string>
#include <vector>

#include <SFML/Graphics.hpp>

#include "constants.hpp"
#include "control.hpp" // TODO remove
#include "input.hpp"

// base class for entries in Menus
class Entry : public InputReader
{
protected:
	sf::Text text;
	float scale;
	bool pending;
public:
	Entry(const std::string& txt, float sz=1.f);

	// is event the kind of event for selecting an entry?
	inline bool is_select_event(const sf::Event& event) const
	{
		return (event.type == sf::Event::KeyReleased && event.key.code == sf::Keyboard::Key::Return) || event.type == sf::Event::MouseButtonReleased;
	}

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

	virtual void draw_on(sf::RenderWindow& window, bool selected);

	virtual bool process_event(sf::Event& event);

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
	MenuSystem& system; // system managing this menu
	Menu* parent; // nullptr for no parent
	sf::Text title;
	std::vector<Entry*> entries;
	int selected = -1; // index of selected entry
	sf::RectangleShape background;
public:
	// create menu managed by sys, with parent p, and title ttl
	Menu(MenuSystem& sys, Menu* p, const std::string& ttl);

	inline Menu* get_parent() const
	{
		return parent;
	}

	// add an entry
	void entry(Entry* ent);

	void update_entries()
	{
		for (auto& ent : entries)
			ent->update();
	}

	// call when Menu or View changes
	void update_position();

	// select entry
	bool select(int index);
	bool select_prev();
	bool select_next();
	// use Entry#bounds to select
	bool select_coords(float x, float y);

	void draw_on(sf::RenderWindow& window) const;

	// navigate menu and pass events to the selected entry
	virtual bool process_event(sf::Event& event);
};

// keeps track of current menu, delegates Events
class MenuSystem : public InputReader
{
	Menu* menu_p = nullptr;

	bool selection_changed = false;
	bool menu_changed = false;
public:
	// use InputReader finished state to determine if menu should be dispayed
	inline void open()
	{
		finished = false;
	}

	inline void close()
	{
		finished = true;
	}

	inline void changed_selection()
	{
		selection_changed = true;
	}

	inline bool selection_was_changed()
	{
		bool t = selection_changed;
		selection_changed = false;
		return t;
	}

	inline bool menu_was_changed()
	{
		bool t = menu_changed;
		menu_changed = false;
		return t;
	}

	// to switch to parent/child menus
	inline void set_menu(Menu& m)
	{
		if (menu_p != nullptr)
			menu_changed = true;

		menu_p = &m;
		menu_p->update_position();
	}

	inline Menu* menu() const
	{
		return menu_p;
	}

	// forward events to current menu
	virtual bool process_event(sf::Event& event);
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
	bool typing {false}; // if input is being handled

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
	virtual bool process_event(sf::Event& event);
	virtual void draw_on(sf::RenderWindow& window, bool selected);
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
	inline unsigned int get_num_choices() const
	{
		return choices.size();
	}
	virtual float get_width() const;
	virtual sf::FloatRect bounds() const;
	void update_choice();
	// left align text, center choice in remaining space
	virtual void set_menu_pos(float center, float width, float top);
	// doesn't need a select
	virtual bool process_event(sf::Event& event);
	virtual void draw_on(sf::RenderWindow& window, bool selected);
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
	bool typing = false; // if input is being handled
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
	virtual bool process_event(sf::Event& event);
	virtual void draw_on(sf::RenderWindow& window, bool selected);
};

#endif

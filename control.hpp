// structs for passing around state
struct State
{
	// general
	sf::RenderWindow* window;
	sf::View* gui_view;
	sf::View* grid_view;
	float zoom; // zoom factor for grid view

	// mouse
	int pos[2]; // last position
	bool update; // signal to update cursor position
	bool mremove; // signal to remove tiles
	int wheel_delta; // amount to zoom
	bool start_selection;
	bool end_selection;
};

// for placing tiles
class Typer : public InputReader
{
	char ch;
public:
	Typer();
	bool get_ch(char* chr);
	virtual bool process_event(sf::Event& event);
};

class MouseControls : public InputReader
{
	State* state;
public:
	MouseControls(State* m);

	virtual bool process_event(sf::Event& event);
};

namespace std
{
	template<> struct less<sf::Event::KeyEvent>
	{
		bool operator() (const sf::Event::KeyEvent& lhs, const sf::Event::KeyEvent& rhs)
		{
			if (lhs.code != rhs.code)
				return lhs.code < rhs.code;
			if (lhs.alt != rhs.alt)
				return !lhs.alt;
			if (lhs.control != rhs.control)
				return !lhs.control;
			if (lhs.shift != rhs.shift)
				return !lhs.shift;
			if (lhs.system != rhs.system)
				return !lhs.system;
			return false;
		}
	};
}

class KeyControls : public InputReader
{
	// for mapping keys to action names
	std::map<sf::Event::KeyEvent, std::string> binds;
	// map action names to current state
	std::map<std::string, bool> pressed;
	// for repeat_t PRESS, map action name to key being released
	std::map<std::string, bool> ready;
	enum repeat_t {PRESS, REPEAT, HOLD};
	// map action name to type of key repeat
	std::map<std::string, repeat_t> repeat;
public:

	KeyControls();
	void bind(const sf::Event::KeyEvent& key, const std::string& str, repeat_t rep);
	void set_defaults();
	bool load_from_file(const std::string& file);
	bool operator[](const std::string& control);
	virtual bool process_event(sf::Event& event);
};

// structs for passing around state
struct State
{
	// general
	sf::RenderWindow* window;
	sf::View* gui_view;
	sf::View* grid_view;
	float zoom; // zoom factor for grid view
	bool switch_controls; // signal to switch control schemes
	bool transpose; // flip selection
	bool center; // center grid

	// keyboard
	char ch; // tile to place

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
	std::map<sf::Event::KeyEvent, std::string> binds;
	std::map<std::string, bool> pressed;
	std::map<std::string, bool> ready;
	enum repeat_t {PRESS, REPEAT, HOLD};
	std::map<std::string, repeat_t> repeat;
public:

	KeyControls();
	void bind(const sf::Event::KeyEvent& key, const std::string& str, repeat_t rep);
	bool load_from_file(const std::string& file);
	bool operator[](const std::string& control);
	virtual bool process_event(sf::Event& event);
};

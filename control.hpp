// TODO comments

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
	std::queue<char> chars;
public:
	Typer();
	bool get_ch(char* ch);
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
	enum repeat_t {PRESS, REPEAT, HOLD};

	class Command
	{
		repeat_t repeat;
	public:
		bool pressed;
		bool ready;
		Command(repeat_t rep = REPEAT);

		inline repeat_t get_repeat() const
		{
			return repeat;
		}
	};

	// for mapping keys to command names
	std::map<sf::Event::KeyEvent, std::string> binds;
	// map command names to commands
	std::map<std::string, Command> commands;
	// map command names to default binds
	std::map<std::string, sf::Event::KeyEvent> defaults;
	void bind(const std::string& command, const std::string& key, repeat_t rep);
public:
	class NotFound : public std::runtime_error
	{
	public:
		NotFound(const std::string& str) : std::runtime_error(str) {}
	};

	KeyControls();

	inline bool has_bind(const std::string& command)
	{
		return commands.find(command) != commands.end();
	}

	void set_defaults();
	void rebind(const sf::Event::KeyEvent& key, const std::string& command);
	void load_from_file(const std::string& filename);
	void write_to_file(const std::string& filename);
	bool operator[](const std::string& control);
	virtual bool process_event(sf::Event& event);
};

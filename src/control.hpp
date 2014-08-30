// for passing around state
struct State
{
	// general
	sf::RenderWindow* window;
	sf::View* grid_view;
	float zoom; // zoom factor for grid view
};

// for classes that handle sf::Events
class InputReader
{
protected:
	bool finished {false};
public:
	virtual ~InputReader() {}

	inline bool is_finished() const
	{
		return finished;
	}

	void reset()
	{
		finished = false;
	}

	virtual bool process_event(sf::Event& event)
	{
		(void)event; // intentionally unused parameter
		return true;
	}
};

// for placing tiles
class Typer : public InputReader
{
	bool single;
	std::queue<char> chars;
public:
	// can make it finish after a single key press
	Typer(bool _single = false);
	// get next tile to place
	bool get_ch(char* ch);
	virtual bool process_event(sf::Event& event);
};

class MouseControls : public InputReader
{
	bool pressed[5] {false, false, false, false, false};
	bool released[5] {false, false, false, false, false};
	bool ready[5] {true, true, true, true, true};
	bool held[5] {false, false, false, false, false};
	bool moved {false};
	int wheel_delta {0};
	sf::Vector2i pos;
public:
	bool was_pressed(unsigned int button);
	bool was_released(unsigned int button);
	inline bool is_held(unsigned int button) const
	{
		return held[button];
	}
	bool was_moved();
	int get_wheel_delta();
	inline const sf::Vector2i& get_pos() const
	{
		return pos;
	}

	virtual bool process_event(sf::Event& event);
};

std::string key2str(const sf::Event::KeyEvent& key);

namespace std
{
	template<> struct less<sf::Event::KeyEvent>
	{
		// for the binds map in KeyControls
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

	template<> struct equal_to<sf::Event::KeyEvent>
	{
		// for the binds map in KeyControls
		bool operator() (const sf::Event::KeyEvent& lhs, const sf::Event::KeyEvent& rhs)
		{
			if (lhs.code != rhs.code)
				return false;
			if (lhs.alt != rhs.alt)
				return false;
			if (lhs.control != rhs.control)
				return false;
			if (lhs.shift != rhs.shift)
				return false;
			if (lhs.system != rhs.system)
				return false;
			return true;
		}
	};
}

// abstraction between keyboard and in-game commands
class KeyControls : public InputReader
{
	// have controls been changed?
	bool changed = false;

	// how holding a key behaves
	enum repeat_t {PRESS, REPEAT, HOLD};

	// stores the state of a command for key repeat purposes
	class Command
	{
		repeat_t repeat;
		bool rebindable;
	public:
		bool pressed {false};
		bool ready {true};

		Command(repeat_t rep = REPEAT, bool rebind = true);

		inline bool is_rebindable() const
		{
			return rebindable;
		}

		inline repeat_t get_repeat() const
		{
			return repeat;
		}

		inline void reset()
		{
			pressed = false;
			ready = true;
		}
	};

	// order of command names
	std::vector<std::string> order;
	// maps keys to command names
	std::map<sf::Event::KeyEvent, std::string> binds;
	// maps command names to commands
	std::map<std::string, Command> commands;
	// maps command names to default binds
	std::map<std::string, sf::Event::KeyEvent> defaults;
	// easily create a command and its default
	void bind(const std::string& command, const std::string& key, repeat_t rep, bool rebindable = true);
	// check if key is allowed to be bound
	bool is_not_bindable(const sf::Event::KeyEvent& key) const;
public:
	// thrown when rebinding a nonexistant action
	class NotFound : public std::runtime_error
	{
	public:
		NotFound(const std::string& str);
	};

	KeyControls();

	inline const std::vector<std::string>& get_order() const
	{
		return order;
	}

	inline const std::map<sf::Event::KeyEvent, std::string>& get_binds() const
	{
		return binds;
	}

	inline bool is_rebindable(const std::string& command) const
	{
		return commands.at(command).is_rebindable();
	}

	// reset the state of a command
	inline void reset(const std::string& command)
	{
		commands[command].reset();
	}

	void set_defaults();
	// create a key binding for an existing command, return if rebind was successful
	bool rebind(const sf::Event::KeyEvent& key, const std::string& command);
	// load from YAML file
	void load_from_file(const std::string& filename);
	// write (non-default) binds to YAML file
	void write_to_file(const std::string& filename);
	// check if a command was invoked
	bool operator[](const std::string& command);

	virtual bool process_event(sf::Event& event);
};

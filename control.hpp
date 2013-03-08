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
	void bind(const sf::Event::KeyEvent& key, const std::string& command, repeat_t rep);
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
	bool load_from_file(const std::string& file);
	bool operator[](const std::string& control);
	virtual bool process_event(sf::Event& event);
};

static const std::vector<std::string> keys
{
	"a",
	"b",
	"c",
	"d",
	"e",
	"f",
	"g",
	"h",
	"i",
	"j",
	"k",
	"l",
	"m",
	"n",
	"o",
	"p",
	"q",
	"r",
	"s",
	"t",
	"u",
	"v",
	"w",
	"x",
	"y",
	"z",
	"num0",
	"num1",
	"num2",
	"num3",
	"num4",
	"num5",
	"num6",
	"num7",
	"num8",
	"num9",
	"escape",
	"lcontrol",
	"lshift",
	"lalt",
	"lsystem",
	"rcontrol",
	"rshift",
	"ralt",
	"rsystem",
	"menu",
	"lbracket",
	"rbracket",
	"semicolon",
	"comma",
	"period",
	"quote",
	"slash",
	"backslash",
	"tilde",
	"equal",
	"dash",
	"space",
	"return",
	"backspace",
	"tab",
	"pageup",
	"pagedown",
	"end",
	"home",
	"insert",
	"delete",
	"add",
	"subtract",
	"multiply",
	"divide",
	"left",
	"right",
	"up",
	"down",
	"numpad0",
	"numpad1",
	"numpad2",
	"numpad3",
	"numpad4",
	"numpad5",
	"numpad6",
	"numpad7",
	"numpad8",
	"numpad9",
	"f1",
	"f2",
	"f3",
	"f4",
	"f5",
	"f6",
	"f7",
	"f8",
	"f9",
	"f10",
	"f11",
	"f12",
	"f13",
	"f14",
	"f15",
	"pause"
};

namespace YAML
{
	template<> struct convert<sf::Event::KeyEvent>
	{
		static bool decode(const Node& node, sf::Event::KeyEvent& key)
		{
			key.alt = false;
			key.control = false;
			key.shift = false;
			key.system = false;

			std::string str = node.as<std::string>();
			for (unsigned int i = 0; i < str.size(); i++)
				str[i] = std::tolower(str[i]);

			// split into substrings
			unsigned int i = 0;
			while (i < str.size() && str[i] == ' ')
				++i;

			if (i == str.size())
			{
				std::cerr << "Empty key binding\n";
				return false;
			}

			std::vector<std::string> subs;

			for (unsigned int j = i + 1; j < str.size(); j++)
			{
				if (str[j] == ' ')
				{
					if (str[i] != ' ')
						subs.push_back(str.substr(i, j - i));
					i = j + 1;
				}
			}

			if (i < str.size())
				subs.push_back(str.substr(i, str.size() - i));

			for (unsigned int i = 0; i < subs.size() - 1; i++)
			{
				if (subs[i] == "alt")
					key.alt = true;
				else if (subs[i] == "ctrl")
					key.control = true;
				else if (subs[i] == "shift")
					key.shift = true;
				else if (subs[i] == "system")
					key.system = true;
				else
				{
					std::cerr << "Unrecognized key modifier: " << subs[i] << std::endl;
					return false;
				}
			}

			i = 0;
			for (; i < keys.size() && keys[i] != subs.back(); i++);
			if (i < keys.size())
				key.code = (sf::Keyboard::Key)i;
			else
			{
				std::cerr << "Unrecognized key: " << subs.back() << std::endl;
				return false;
			}

			return true;
		}
	};
}

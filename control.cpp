#include "bananagrams.hpp"

using std::cerr;
using std::endl;
using std::string;

Typer::Typer() {}

bool Typer::get_ch(char* ch)
{
	if (!chars.empty())
	{
		*ch = chars.front();
		chars.pop();
		return true;
	}
	return false;
}

bool Typer::process_event(sf::Event& event)
{
	// add character to queue (if a letter was pressed)
	if (event.type == sf::Event::KeyPressed && event.key.code >= sf::Keyboard::Key::A && event.key.code <= sf::Keyboard::Key::Z)
		chars.push(event.key.code - sf::Keyboard::Key::A + 'A');
	return true;
}

MouseControls::MouseControls(State* m)
{
	state = m;
}

bool MouseControls::process_event(sf::Event& event)
{
	switch(event.type)
	{
		case sf::Event::MouseButtonPressed:
			if (event.mouseButton.button == sf::Mouse::Left)
				state->start_selection = true;
			else if (event.mouseButton.button == sf::Mouse::Right)
				state->mremove = true;
			break;
		case sf::Event::MouseButtonReleased:
			state->update = true;
			if (event.mouseButton.button == sf::Mouse::Left)
				state->end_selection = true;
			else if (event.mouseButton.button == sf::Mouse::Right)
				state->mremove = false;
			break;
		case sf::Event::MouseMoved:
			{
				state->update = true;
				state->pos[0] = event.mouseMove.x;
				state->pos[1] = event.mouseMove.y;
			}
			break;
		case sf::Event::MouseWheelMoved:
			state->wheel_delta = event.mouseWheel.delta;
			break;
		default:
			break;
	}

	return true;
}

// for converting key names to sf::Keyboard::Key values
static const std::vector<string> keys
{
	"a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o",
	"p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "num0", "num1",
	"num2", "num3", "num4", "num5", "num6", "num7", "num8", "num9", "escape",
	"lcontrol", "lshift", "lalt", "lsystem", "rcontrol", "rshift", "ralt",
	"rsystem", "menu", "lbracket", "rbracket", "semicolon", "comma", "period",
	"quote", "slash", "backslash", "tilde", "equal", "dash", "space", "return",
	"backspace", "tab", "pageup", "pagedown", "end", "home", "insert",
	"delete", "add", "subtract", "multiply", "divide", "left", "right", "up",
	"down", "numpad0", "numpad1", "numpad2", "numpad3", "numpad4", "numpad5",
	"numpad6", "numpad7", "numpad8", "numpad9", "f1", "f2", "f3", "f4", "f5",
	"f6", "f7", "f8", "f9", "f10", "f11", "f12", "f13", "f14", "f15", "pause"
};

sf::Event::KeyEvent str2key(const string& strn)
{
	sf::Event::KeyEvent key;

	// key code if we can't find it in the keys array
	key.code = sf::Keyboard::Key::Unknown;
	key.alt = false;
	key.control = false;
	key.shift = false;
	key.system = false;

	string str = strn;
	for (unsigned int i = 0; i < str.size(); i++)
		str[i] = std::tolower(str[i]);

	// split into substrings
	unsigned int i = 0;
	for (; i < str.size() && std::isspace(str[i]); ++i);

	if (i == str.size())
		return key;

	std::vector<string> subs;

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

	// parse key modifiers
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
		else // invalid modifier
			return key;
	}

	// find matching key value
	for (i = 0; i < keys.size() && keys[i] != subs.back(); i++);
	if (i < keys.size())
		key.code = (sf::Keyboard::Key)i;

	return key;
}

string key2str(const sf::Event::KeyEvent& key)
{
	// TODO there is an issue with SFML and modifiers with some non-character keys (e.g. ?)
	string str = keys[key.code];
	if (key.system)
		str = "system " + str;
	if (key.shift)
		str = "shift " + str;
	if (key.control)
		str = "ctrl " + str;
	if (key.alt)
		str = "alt " + str;
	return str;
}

KeyControls::Command::Command(repeat_t rep, bool rebind)
{
	repeat = rep;
	rebindable = rebind;
	pressed = false;
	ready = true;
}

KeyControls::KeyControls()
{
	bind("left"           , "left"           , REPEAT);
	bind("right"          , "right"          , REPEAT);
	bind("up"             , "up"             , REPEAT);
	bind("down"           , "down"           , REPEAT);
	bind("left_fast"      , "shift left"     , REPEAT);
	bind("right_fast"     , "shift right"    , REPEAT);
	bind("up_fast"        , "shift up"       , REPEAT);
	bind("down_fast"      , "shift down"     , REPEAT);
	bind("remove"         , "backspace"      , REPEAT);
	bind("zoom_in"        , "ctrl up"        , HOLD  );
	bind("zoom_out"       , "ctrl down"      , HOLD  );
	bind("zoom_in_fast"   , "ctrl shift up"  , HOLD  );
	bind("zoom_out_fast"  , "ctrl shift down", HOLD  );
	bind("quick_place"    , "lcontrol"       , HOLD  );
	bind("menu"           , "escape"         , PRESS );
	bind("peel"           , "space"          , PRESS );
	bind("center"         , "ctrl c"         , PRESS );
	bind("dump"           , "ctrl d"         , PRESS );
	bind("cut"            , "ctrl x"         , PRESS );
	bind("paste"          , "ctrl p"         , PRESS );
	bind("flip"           , "ctrl f"         , PRESS );
	bind("scramble_tiles" , "f1"             , PRESS );
	bind("sort_tiles"     , "f2"             , PRESS );
	bind("count_tiles"    , "f3"             , PRESS );
	bind("stack_tiles"    , "f4"             , PRESS );
	set_defaults();
}

void KeyControls::bind(const string& command, const string& key, repeat_t rep)
{
	commands[command] = Command(rep);
	defaults[command] = str2key(key);
}

void KeyControls::rebind(const sf::Event::KeyEvent& key, const string& command)
{
	if (commands.find(command) == commands.end())
		throw NotFound(command);
	for (auto pair = binds.begin(); pair != binds.end(); ++pair)
		if (pair->second == command)
			binds.erase(pair);
	binds[key] = command;

	commands[command].pressed = false;
	commands[command].ready = true;
}

void KeyControls::set_defaults()
{
	binds.clear();
	for (auto pair : defaults)
		binds[pair.second] = pair.first;
}

void KeyControls::load_from_file(const string& filename)
{
	YAML::Node bindings;
	try
	{
		bindings = YAML::LoadFile(filename);
	}
	catch (YAML::BadFile)
	{
		cerr << "Can't read " << filename << endl;
		return;
	}

	if (!bindings.IsMap())
	{
		cerr << "Ignoring bad config file\n";
		return;
	}

	for (auto binding : bindings)
	{
		try
		{
			rebind(binding.second.as<sf::Event::KeyEvent>(), binding.first.as<string>());
		}
		catch (NotFound)
		{
			cerr << "Unrecognized command: " << binding.first.as<string>() << endl;
		}
		catch (YAML::TypedBadConversion<sf::Event::KeyEvent>)
		{
			cerr << "Unrecognized key combination: " << binding.second.as<string>() << endl;
		}
		catch (YAML::TypedBadConversion<string>)
		{
			cerr << "Empty binding: " << binding.first.as<string>() << endl;
		}
	}
}

void KeyControls::write_to_file(const string& filename)
{
	std::ofstream config(filename, std::ios_base::out);
	if (!config.is_open())
	{
		cerr << "Couldn't write to " << filename << endl;
		return;
	}

	YAML::Emitter out;

	out << YAML::Comment("key bindings");
	out << YAML::BeginMap;

	for (auto pair : binds)
	{
		// if bind is non-default
		if (std::less<sf::Event::KeyEvent>()(pair.first, defaults[pair.second]) || std::less<sf::Event::KeyEvent>()(defaults[pair.second], pair.first))
			out << YAML::Key << pair.second
			    << YAML::Value << YAML::Node(pair.first)
			    << YAML::Newline;
	}

	out << YAML::EndMap;

	config << out.c_str();
	config.close();
}

bool KeyControls::operator[](const string& command)
{
	Command& c = commands[command];
	bool press = c.pressed;

	if (c.get_repeat() != HOLD)
		c.pressed = false;

	return press;
}

bool KeyControls::process_event(sf::Event& event)
{
	if (event.type == sf::Event::KeyPressed)
	{
		auto it = binds.find(event.key);
		if (it != binds.end())
		{
			Command& c = commands[it->second];
			switch (c.get_repeat())
			{
				case PRESS:
					if (c.ready)
					{
						c.pressed = true;
						c.ready = false;
					}
					break;
				case REPEAT:
				case HOLD:
					c.pressed = true;
					break;
			}

			// don't continue bound keypresses
			return false;
		}
	}
	else if (event.type == sf::Event::KeyReleased)
	{
		// for modifiers, need to disable any HOLD binds relying on them
		switch (event.key.code)
		{
			case sf::Keyboard::Key::LAlt:
			case sf::Keyboard::Key::RAlt:
				for (auto pair : binds)
					if (commands[pair.second].get_repeat() == HOLD && (pair.first.alt || pair.first.code == sf::Keyboard::Key::LAlt || pair.first.code == sf::Keyboard::Key::RAlt))
						commands[pair.second].pressed = false;
				break;
			case sf::Keyboard::Key::LControl:
			case sf::Keyboard::Key::RControl:
				for (auto pair : binds)
					if (commands[pair.second].get_repeat() == HOLD && (pair.first.control || pair.first.code == sf::Keyboard::Key::LControl || pair.first.code == sf::Keyboard::Key::RControl))
						commands[pair.second].pressed = false;
				break;
			case sf::Keyboard::Key::LShift:
			case sf::Keyboard::Key::RShift:
				for (auto pair : binds)
					if (commands[pair.second].get_repeat() == HOLD && (pair.first.shift || pair.first.code == sf::Keyboard::Key::LShift || pair.first.code == sf::Keyboard::Key::RShift))
						commands[pair.second].pressed = false;
				break;
			case sf::Keyboard::Key::LSystem:
			case sf::Keyboard::Key::RSystem:
				for (auto pair : binds)
					if (commands[pair.second].get_repeat() == HOLD && (pair.first.system || pair.first.code == sf::Keyboard::Key::LSystem || pair.first.code == sf::Keyboard::Key::RSystem))
						commands[pair.second].pressed = false;
				break;
			default:
				break;
		}
		auto it = binds.find(event.key);
		if (it != binds.end())
		{
			Command& c = commands[it->second];
			switch (c.get_repeat())
			{
				case PRESS:
					c.ready = true;
					break;
				case REPEAT:
					break;
				case HOLD:
					c.pressed = false;
					break;
			}
		}
	}

	return true;
}

namespace YAML
{
	template<> struct convert<sf::Event::KeyEvent>
	{
		static Node encode(const sf::Event::KeyEvent& key)
		{
			return Node(key2str(key));
		}

		static bool decode(const Node& node, sf::Event::KeyEvent& key)
		{
			key = str2key(node.as<string>());

			if (key.code == sf::Keyboard::Key::Unknown)
				return false;

			return true;
		}
	};
}

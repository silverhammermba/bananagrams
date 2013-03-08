#include "bananagrams.hpp"

using std::cerr;
using std::endl;
using std::string;

Typer::Typer()
{
	ch = 'A' - 1;
}

bool Typer::get_ch(char* chr)
{
	if (ch >= 'A' && ch <= 'Z')
	{
		*chr = ch;
		ch = 'A' - 1;
		return true;
	}
	return false;
}

bool Typer::process_event(sf::Event& event)
{
	if (event.type == sf::Event::KeyPressed && event.key.code >= sf::Keyboard::Key::A && event.key.code <= sf::Keyboard::Key::Z)
		ch = event.key.code - sf::Keyboard::Key::A + 'A';
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

KeyControls::Command::Command(repeat_t rep)
{
	pressed = false;
	ready = true;
	repeat = rep;
}

KeyControls::KeyControls()
{
	set_defaults();
}

void KeyControls::bind(const sf::Event::KeyEvent& key, const string& command, repeat_t rep)
{
	binds[key] = command;
	commands[command] = Command(rep);
}

void KeyControls::rebind(const sf::Event::KeyEvent& key, const string& command)
{
	if (!has_bind(command))
		throw NotFound(command);
	binds[key] = command;
	commands[command].pressed = false;
	commands[command].ready = true;
}

void KeyControls::set_defaults()
{
	// TODO is there a better way to structure these?
	binds.clear();
	commands.clear();

	sf::Event::KeyEvent key;
	key.alt = false;
	key.control = false;
	key.shift = false;
	key.system = false;

	key.code = sf::Keyboard::Key::Left;
	bind(key, "left", REPEAT);
	key.code = sf::Keyboard::Right;
	bind(key, "right", REPEAT);
	key.code = sf::Keyboard::Up;
	bind(key, "up", REPEAT);
	key.code = sf::Keyboard::Down;
	bind(key, "down", REPEAT);
	key.shift = true;
	key.code = sf::Keyboard::Left;
	bind(key, "left_fast", REPEAT);
	key.code = sf::Keyboard::Right;
	bind(key, "right_fast", REPEAT);
	key.code = sf::Keyboard::Up;
	bind(key, "up_fast", REPEAT);
	key.code = sf::Keyboard::Down;
	bind(key, "down_fast", REPEAT);
	key.control = true;
	key.shift = false;
	key.code = sf::Keyboard::Key::Up;
	bind(key, "zoom_in", HOLD);
	key.code = sf::Keyboard::Key::Down;
	bind(key, "zoom_out", HOLD);
	key.shift = true;
	key.code = sf::Keyboard::Key::Up;
	bind(key, "zoom_in_fast", HOLD);
	key.code = sf::Keyboard::Key::Down;
	bind(key, "zoom_out_fast", HOLD);
	key.control = false;
	key.shift = false;
	key.code = sf::Keyboard::BackSpace;
	bind(key, "remove", REPEAT);
	key.code = sf::Keyboard::Space;
	bind(key, "peel", PRESS);
	key.control = true;
	key.code = sf::Keyboard::C;
	bind(key, "center", PRESS);
	key.code = sf::Keyboard::D;
	bind(key, "dump", PRESS);
	key.code = sf::Keyboard::X;
	bind(key, "cut", PRESS);
	key.code = sf::Keyboard::V;
	bind(key, "paste", PRESS);
	key.code = sf::Keyboard::F;
	bind(key, "flip", PRESS);
	key.control = false;
	key.code = sf::Keyboard::LControl;
	bind(key, "quick_place", HOLD);
}

bool KeyControls::load_from_file(const string& filename)
{
	YAML::Node bindings = YAML::LoadFile(filename);

	if (!bindings.IsMap())
	{
		cerr << "Ignoring bad config file\n"
		return false;
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
			// YAML conversion already prints errors
		}
	}

	return true;
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
		// TODO this doesn't work if you release modifier keys in different orders
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

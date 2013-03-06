#include "bananagrams.hpp"

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

KeyControls::KeyControls()
{
	// set up default binds
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
}

void KeyControls::bind(const sf::Event::KeyEvent& key, const std::string& str, repeat_t rep)
{
	auto it = binds.find(key);
	if (it != binds.end())
		binds.erase(it);

	binds[key] = str;
	pressed[str] = false;
	ready[str] = true;
	repeat[str] = rep;
}

bool KeyControls::load_from_file(const std::string& file)
{
	return false;
}

bool KeyControls::operator[](const std::string& control)
{
	bool press = pressed[control];

	if (repeat[control] != HOLD)
		pressed[control] = false;

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
			auto action = it->second;
			switch (repeat[action])
			{
				case PRESS:
					if (ready[action])
					{
						pressed[action] = true;
						ready[action] = false;
					}
					break;
				case REPEAT:
				case HOLD:
					pressed[action] = true;
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
					if (pair.first.alt && repeat[pair.second] == HOLD)
						pressed[pair.second] = false;
				break;
			case sf::Keyboard::Key::LControl:
			case sf::Keyboard::Key::RControl:
				for (auto pair : binds)
					if (pair.first.control && repeat[pair.second] == HOLD)
						pressed[pair.second] = false;
				break;
			case sf::Keyboard::Key::LShift:
			case sf::Keyboard::Key::RShift:
				for (auto pair : binds)
					if (pair.first.shift && repeat[pair.second] == HOLD)
						pressed[pair.second] = false;
				break;
			case sf::Keyboard::Key::LSystem:
			case sf::Keyboard::Key::RSystem:
				for (auto pair : binds)
					if (pair.first.system && repeat[pair.second] == HOLD)
						pressed[pair.second] = false;
				break;
			default:
				break;
		}
		auto it = binds.find(event.key);
		if (it != binds.end())
		{
			auto action = it->second;
			switch (repeat[action])
			{
				case PRESS:
					ready[action] = true;
					break;
				case REPEAT:
					break;
				case HOLD:
					pressed[action] = false;
					break;
			}
		}
	}

	return true;
}

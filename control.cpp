#include <SFML/Graphics.hpp>
#include "bananagrams.hpp"
#include "control.hpp"

MouseControls::MouseControls(State* m)
{
	state = m;
}

bool MouseControls::process_event(const sf::Event& event)
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

SimpleControls::SimpleControls(State* s)
{
	state = s;
}

bool SimpleControls::process_event(const sf::Event& event)
{
	if (event.type == sf::Event::KeyPressed)
	{
		switch (event.key.code)
		{
			case sf::Keyboard::Left:
				state->delta.x = -1;
				break;
			case sf::Keyboard::Right:
				state->delta.x = 1;
				break;
			case sf::Keyboard::Up:
				state->delta.y = -1;
				break;
			case sf::Keyboard::Down:
				state->delta.y = 1;
				break;
			case sf::Keyboard::Home:
				state->center = true;
				break;
			case sf::Keyboard::LControl:
			case sf::Keyboard::RControl:
				state->ctrl = true;
				break;
			case sf::Keyboard::LShift:
			case sf::Keyboard::RShift:
				state->sprint = true;
				break;
			case sf::Keyboard::BackSpace:
				state->kremove = true;
				break;
			case sf::Keyboard::Space:
				state->peel = true;
				break;
			case sf::Keyboard::D:
				if (state->ctrl)
				{
					state->dump = true;
					return true;
				}
				break;
			case sf::Keyboard::F:
				if (state->ctrl)
				{
					state->transpose = true;
					return true;
				}
				break;
			case sf::Keyboard::X:
				if (state->ctrl)
				{
					state->cut = true;
					return true;
				}
				break;
			case sf::Keyboard::V:
				if (state->ctrl)
				{
					state->paste = true;
					return true;
				}
				break;
			default:
				break;
		}
		if (sf::Keyboard::A <= event.key.code && event.key.code <= sf::Keyboard::Z)
			state->ch = event.key.code - sf::Keyboard::A + 'A';
	}
	else if (event.type == sf::Event::KeyReleased)
	{
		switch (event.key.code)
		{
			case sf::Keyboard::Left:
			case sf::Keyboard::Right:
				state->delta.x = 0;
				break;
			case sf::Keyboard::Up:
			case sf::Keyboard::Down:
				state->delta.y = 0;
				break;
			case sf::Keyboard::LControl:
			case sf::Keyboard::RControl:
				state->ctrl = false;
				break;
			case sf::Keyboard::LShift:
			case sf::Keyboard::RShift:
				state->sprint = false;
				break;
			default:
				break;
		}
	}
	return true;
}

VimControls::VimControls(State* s)
{
	state = s;
}

bool VimControls::process_event(const sf::Event& event)
{
	if (event.type == sf::Event::KeyPressed)
	{
		// return to skip processing letter key as insert
		switch (event.key.code)
		{
			// directions
			case sf::Keyboard::Y:
				if (state->ctrl)
				{
					state->cut = true;
					return true;
				}
				if (shift)
					state->sprint = true;
				else
					break;
			case sf::Keyboard::H:
				if (!shift)
					break;
			case sf::Keyboard::Left:
				state->delta.x = -1;
				return true;
			case sf::Keyboard::O:
				if (shift)
					state->sprint = true;
				else
					break;
			case sf::Keyboard::L:
				if (!shift)
					break;
			case sf::Keyboard::Right:
				state->delta.x = 1;
				return true;
			case sf::Keyboard::I:
				if (shift)
					state->sprint = true;
				else
					break;
			case sf::Keyboard::K:
				if (!(shift || state->ctrl))
					break;
			case sf::Keyboard::Up:
				state->delta.y = -1;
				return true;
			case sf::Keyboard::U:
				if (shift)
					state->sprint = true;
				else
					break;
			case sf::Keyboard::J:
				if (!(shift || state->ctrl))
					break;
			case sf::Keyboard::Down:
				state->delta.y = 1;
				return true;
			// special functions
			case sf::Keyboard::F:
				if (shift)
				{
					state->transpose = true;
					return true;
				}
			case sf::Keyboard::P:
				if (shift || state->ctrl)
				{
					state->paste = true;
					return true;
				}
				break;
			// modifier keys
			case sf::Keyboard::LControl:
			case sf::Keyboard::RControl:
				state->ctrl = true;
				break;
			case sf::Keyboard::LShift:
			case sf::Keyboard::RShift:
				shift = true;
				break;
			case sf::Keyboard::X:
				if (!shift)
					break;
			case sf::Keyboard::BackSpace:
				state->kremove = true;
				return true;
			case sf::Keyboard::D:
				if (!shift)
					break;
				state->dump = true;
				return true;
			case sf::Keyboard::Space:
				state->peel = true;
				break;
			default:
				break;
		}
		if (sf::Keyboard::A <= event.key.code && event.key.code <= sf::Keyboard::Z)
			state->ch = event.key.code - sf::Keyboard::A + 'A';
	}
	else if (event.type == sf::Event::KeyReleased)
	{
		switch (event.key.code)
		{
			//YUIO
			//HJKL
			case sf::Keyboard::Y:
			case sf::Keyboard::O:
				state->sprint = false;
			case sf::Keyboard::H:
			case sf::Keyboard::L:
			case sf::Keyboard::Left:
			case sf::Keyboard::Right:
				state->delta.x = 0;
				break;
			case sf::Keyboard::U:
			case sf::Keyboard::I:
				state->sprint = false;
			case sf::Keyboard::J:
			case sf::Keyboard::K:
			case sf::Keyboard::Up:
			case sf::Keyboard::Down:
				state->delta.y = 0;
				break;
			case sf::Keyboard::LControl:
			case sf::Keyboard::RControl:
				state->ctrl = false;
				if (!shift)
					state->delta.y = 0;
				break;
			case sf::Keyboard::LShift:
			case sf::Keyboard::RShift:
				shift = false;
				state->sprint = false;
				state->delta.x = 0;
				state->delta.y = 0;
				break;
			default:
				break;
		}
	}
	return true;
}

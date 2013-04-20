#include "client.hpp"

using std::string;

Game::~Game()
{
	end();
}

bool Game::load(const std::string& filename)
{
	// TODO
	return false;
}

void Game::save(const std::string& filename)
{
	// TODO
}

void Game::clear_buffer()
{
	if (buffer != nullptr)
		delete buffer;
	buffer = nullptr;
}

void Game::end()
{
	for (auto tile : bunch)
		delete tile;
	bunch.clear();

	grid.clear();

	hand.clear();

	dictionary.clear();

	clear_buffer();

	messages.clear();
}

void Game::update_mouse_pos(const sf::RenderWindow& window, const sf::View& view, const sf::Vector2i& pos)
{
	sf::Vector2u w_size = window.getSize();
	sf::Vector2f size = view.getSize();
	sf::Vector2f center = view.getCenter();

	// update mouse cursor position
	mcursor.set_pos({(int)std::floor(((pos.x * size.x) / w_size.x + center.x - (size.x / 2)) / PPB), (int)std::floor(((pos.y * size.y) / w_size.y + center.y - (size.y / 2)) / PPB)});

	if (buffer != nullptr)
		buffer->set_pos(mcursor.get_pos());
}

// start a selection at the mouse cursor
void Game::select()
{
	selecting = true;
	selected = false;
	sel1 = mcursor.get_pos();
}

// resize the current selection to the mouse cursor
void Game::resize_selection()
{
	sel2 = mcursor.get_pos();
	// update selection rect
	selection.set_size({(unsigned int)std::abs(sel1.x - sel2.x) + 1, (unsigned int)std::abs(sel1.y - sel2.y) + 1});
	selection.set_pos({std::min(sel1.x, sel2.x), std::min(sel1.y, sel2.y)});
}

// finish selection, move cursor (if necessary), return selection size
unsigned int Game::complete_selection()
{
	selecting = false;
	selected = true;

	sf::Vector2u size = selection.get_size();
	unsigned int tiles = size.x * size.y;

	// if the selection was only 1 square, move cursor
	if (tiles == 1)
	{
		selected = false;
		cursor.set_pos(mcursor.get_pos());
		last_move = ZERO;
	}

	return tiles;
}

// place tile at cursor if only one letter remaining
void Game::quick_place()
{
	// look for remaining tiles
	char last {'A' - 1};
	for (char ch = 'A'; ch <= 'Z'; ch++)
	{
		if (hand.has_any(ch))
		{
			if (last < 'A')
				last = ch;
			else
			{
				last = 'Z' + 1;
				break;
			}
		}
	}

	if (last < 'A')
		messages.add("You do not have any tiles.", Message::Severity::LOW);
	else if (last > 'Z')
		messages.add("You have too many letters to place using the mouse.", Message::Severity::HIGH);
	else
	{
		Tile* tile {grid.swap(cursor.get_pos(), hand.remove_tile(last))};

		if (tile != nullptr)
			hand.add_tile(tile);
	}
}

void Game::cut()
{
	if (buffer == nullptr)
	{
		if (selected)
		{
			buffer = new CutBuffer(grid, std::min(sel1.x, sel2.x), std::min(sel1.y, sel2.y), selection.get_size());

			if (buffer->is_empty())
			{
				messages.add("Nothing selected.", Message::Severity::LOW);
				clear_buffer();
			}
		}
		else
			messages.add("Nothing selected.", Message::Severity::LOW);

		selected = false;
	}
	else
	{
		buffer->clear(hand);
		clear_buffer();
		messages.add("Added cut tiles back to your hand.", Message::Severity::LOW);
	}
}

void Game::flip_buffer()
{
	if (buffer != nullptr)
		buffer->transpose();
}

void Game::paste()
{
	if (buffer != nullptr)
	{
		buffer->paste(grid, hand);
		clear_buffer();
	}
	else
		messages.add("Cannot paste: no tiles were cut.", Message::Severity::LOW);
}

void Game::remove_at_mouse()
{
	// remove tile
	Tile* tile {grid.remove(mcursor.get_pos())};
	if (tile != nullptr)
		hand.add_tile(tile);
}

void Game::dump()
{
	if (bunch.size() >= 3)
	{
		Tile* dumped {grid.remove(cursor.get_pos())};
		if (dumped == nullptr)
			messages.add("You need to select a tile to dump.", Message::Severity::LOW);
		else
		{
			// take three
			for (unsigned int i = 0; i < 3; i++)
			{
				Tile* tile {bunch.back()};
				bunch.pop_back();
				hand.add_tile(tile);
			}

			random_insert<Tile*>(bunch, dumped);
		}
	}
	else
		messages.add("There are not enough tiles left to dump!", Message::Severity::HIGH);
}

void Game::peel()
{
	bool spent {true};
	for (char ch = 'A'; ch <= 'Z'; ch++)
		if (hand.has_any(ch))
		{
			spent = false;
			break;
		}
	if (spent)
	{
		std::vector<string> mess;
		if (grid.is_valid(dictionary, mess))
		{
			if (bunch.size() > 0)
			{
				Tile* tile {bunch.back()};
				bunch.pop_back();
				hand.add_tile(tile);
				for (auto message : mess)
					messages.add(message, Message::Severity::LOW);
			}
			else
				messages.add("You win!", Message::Severity::LOW);
		}
		else
			for (auto message : mess)
				messages.add(message, Message::Severity::HIGH);
	}
	else
		messages.add("You have not used all of your letters.", Message::Severity::HIGH);
}

void Game::remove()
{
	const sf::Vector2i& pos = cursor.get_pos();
	// if the cursor is ahead of the last added character, autoadvance
	if (pos == last_place + X)
	{
		cursor.set_pos(last_place);
		last_place -= X;
	}
	else if (pos == last_place + Y)
	{
		cursor.set_pos(last_place);
		last_place -= Y;
	}
	// else if you are not near the last character and the space is empty, try to autoadvance
	else if (grid.get(pos) == nullptr)
	{
		sf::Vector2i next {0, 0};

		// if right of a tile
		if (grid.get(pos - X) != nullptr)
			next = X;
		// if below a tile
		else if (grid.get(pos - Y) != nullptr)
			next = Y;
		else
			next = last_move;

		// if all else fails
		if (next == ZERO)
			next = X;

		cursor.move(-next);
		last_place = cursor.get_pos() - next;
	}
	// else just remove the current character, figure out autoadvance next time

	Tile* tile {grid.remove(cursor.get_pos())};
	if (tile != nullptr)
		hand.add_tile(tile);
}

void Game::place(char ch)
{
	bool placed {false};

	// if space is empty or has a different letter
	if (grid.get(cursor.get_pos()) == nullptr || grid.get(cursor.get_pos())->ch() != ch)
	{
		if (hand.has_any(ch))
		{
			Tile* tile {grid.swap(cursor.get_pos(), hand.remove_tile(ch))};

			if (tile != nullptr)
				hand.add_tile(tile);
			placed = true;
		}
	}
	else // space already has the letter to be placed
		placed = true;

	// if we placed a letter, try to autoadvance
	if (placed)
	{
		sf::Vector2i next {0, 0};
		if (cursor.get_pos() == last_place + X)
			next.x = 1;
		else if (cursor.get_pos() == last_place + Y)
			next.y = 1;
		else if (grid.get(cursor.get_pos() - X) != nullptr)
			next.x = 1;
		else if (grid.get(cursor.get_pos() - Y) != nullptr)
			next.y = 1;
		else if (grid.get(cursor.get_pos() + X) != nullptr)
			next.x = 1;
		else if (grid.get(cursor.get_pos() + Y) != nullptr)
			next.y = 1;
		else
			next = last_move;

		// if all else fails
		if (next == ZERO)
			next = X;

		last_place = cursor.get_pos();
		cursor.move(next);
	}
	else
		messages.add("You are out of " + string(1, ch) + "s!", Message::Severity::HIGH);
}

void Game::update_cursor(const sf::View& view)
{
	if (!set_view_to_cursor)
	{
		// move cursor if zooming in moves it off screen
		sf::Vector2f center = view.getCenter();
		sf::Vector2f gsize = view.getSize();
		sf::Vector2f spos = cursor.get_center();
		while (spos.x - center.x > gsize.x / 4)
		{
			cursor.move(-X);
			spos.x -= PPB;
			last_move = ZERO;
		}

		while (spos.x - center.x < gsize.x / -4)
		{
			cursor.move(X);
			spos.x += PPB;
			last_move = ZERO;
		}

		while (spos.y - center.y > gsize.y / 4)
		{
			cursor.move(-Y);
			spos.y -= PPB;
			last_move = ZERO;
		}

		while (spos.y - center.y < gsize.y / -4)
		{
			cursor.move(Y);
			spos.y += PPB;
			last_move = ZERO;
		}

		set_view_to_cursor = true;
	}
}

void Game::set_zoom(float zoom)
{
	cursor.set_zoom(zoom);
	mcursor.set_zoom(zoom);
	selection.set_zoom(zoom);
}

// TODO make this const once Hand is fixed
void Game::draw_on(sf::RenderWindow& window, const sf::View& grid_view, const sf::View& gui_view)
{
	window.setView(grid_view);
	grid.draw_on(window);
	if (selecting || selected)
		selection.draw_on(window);
	if (buffer != nullptr)
		buffer->draw_on(window);
	cursor.draw_on(window);
	mcursor.draw_on(window);

	window.setView(gui_view);
	messages.draw_on(window);
	hand.draw_on(window);
}

bool Game::start_singleplayer(const std::string& dict, int multiplier, int divider)
{
	// TODO validate somehow
	// TODO cache so we don't have to reload every time
	std::ifstream words(dict);
	if (!words.is_open())
	{
		std::cerr << "Couldn't find " << dict << "!\n";
		return false;
	}

	end();

	// parse dictionary
	string line;
	while (std::getline(words, line))
	{
		auto pos = line.find_first_of(' ');
		if (pos == string::npos)
			dictionary[line] = "";
		else
			dictionary[line.substr(0, pos)] = line.substr(pos + 1, string::npos);
	}
	words.close();

	// create tiles for the bunch
	for (char ch = 'A'; ch <= 'Z'; ++ch)
		for (unsigned int i = 0; i < ((letter_count[ch - 'A'] * multiplier) / divider); ++i)
			random_insert(bunch, new Tile(ch));

	// take tiles from the bunch for player
	for (unsigned int i = 0; i < 21; i++)
	{
		auto tile = bunch.back();
		bunch.pop_back();
		hand.add_tile(tile);
	}

	selected = false;
	selecting = false;

	return true;
}

bool Game::start_multiplayer(const std::string& ip, unsigned short server_port, const std::string& name)
{
	unsigned short client_port = server_port + 1;
	// TODO catch errors here
	sf::IpAddress server_ip {ip};
	socket.bind(client_port);

	sf::Packet join;
	join << sf::Uint8(0) << name;

	socket.send(join, server_ip, server_port);
	socket.unbind();
	return false;
}

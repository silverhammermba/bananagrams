#include "client.hpp"

using std::cerr;
using std::endl;
using std::string;

Game::~Game()
{
	grid.clear();

	hand.clear();

	clear_buffer();

	messages.clear();
}

void Game::step(float time)
{
	grid.step(time);
	messages.step(time);
}

void Game::clear_buffer()
{
	if (buffer != nullptr)
		delete buffer;
	buffer = nullptr;
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

void Game::draw_on(sf::RenderWindow& window, const sf::View& grid_view, const sf::View& gui_view) const
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

// check if hand is empty and grid is continuous
bool Game::peel()
{
	if (!hand.is_empty())
	{
		messages.add("You have not used all of your letters.", Message::Severity::HIGH);
		return false;
	}

	if (!grid.is_continuous())
	{
		messages.add("Your tiles are not all connected.", Message::Severity::HIGH);
		return false;
	}

	return true;
}

SingleplayerGame::SingleplayerGame(const std::string& dict, int multiplier, int divider)
{
	// TODO validate somehow
	// TODO cache so we don't have to reload every time
	std::ifstream words(dict);

	if (words.is_open())
	{
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
	}
	else
		messages.add("Failed to load dictionary '" + dict + "'", Message::Severity::CRITICAL);
}

SingleplayerGame::~SingleplayerGame()
{
	for (auto tile : bunch)
		delete tile;
	bunch.clear();

	dictionary.clear();
}

bool SingleplayerGame::load(const std::string& filename)
{
	// TODO
	return false;
}

void SingleplayerGame::save(const std::string& filename)
{
	// TODO
}

void SingleplayerGame::dump()
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

bool SingleplayerGame::peel()
{
	if (!Game::peel())
		return false;

	auto words = grid.get_words();
	bool valid {true};

	// check words
	for (auto word : words)
	{
		if (dictionary.find(word.first) == dictionary.end())
		{
			valid = false;
			messages.add(word.first + " is not a word.", Message::Severity::HIGH);
			// color incorrect tiles
			for (auto& pos: word.second)
				grid.bad_word(pos[0], pos[1], pos[2]);
		}
	}

	if (!valid)
		return false;

	if (bunch.size() > 0)
	{
		Tile* tile {bunch.back()};
		bunch.pop_back();
		hand.add_tile(tile);
	}
	else
		messages.add("You win!", Message::Severity::CRITICAL);

	return true;
}

MultiplayerGame::MultiplayerGame(const std::string& server, const std::string& name)
	: server_port {default_server_port}, id {boost::uuids::to_string(boost::uuids::random_generator()())}
{
	std::string ip {server};

	// process server string
	// TODO make this a little more robust...
	size_t port_p {server.find(':')};
	if (port_p != std::string::npos)
	{
		std::stringstream port_s;
		port_s << server.substr(port_p + 1);
		port_s >> server_port;

		ip = server.substr(0, port_p);
	}

	server_ip = sf::IpAddress(ip);

	unsigned short client_port = server_port + 1;
	// TODO catch errors here
	socket.bind(client_port);
	socket.setBlocking(false);

	sf::Packet join;
	join << cl_connect << id << protocol_version << name;

	set_ack(join, sv_connect);

	messages.add("Connecting to " + server + "...", Message::Severity::CRITICAL);
}

MultiplayerGame::~MultiplayerGame()
{
	sf::Packet disconnect;
	disconnect << cl_disconnect << id;

	socket.send(disconnect, server_ip, server_port);

	socket.unbind();
}

void MultiplayerGame::step(float time)
{
	Game::step(time);

	if (ack != nullptr)
		ack->age(time);

	sf::Packet packet;
	sf::IpAddress ip;
	unsigned short port;
	if (socket.receive(packet, ip, port) == sf::Socket::Status::Done)
	{
		cerr << "Received packet from " << ip << ":" << port << endl;
		// TODO somehow verify that this is actually the server...
		process_packet(packet);
	}
}

void MultiplayerGame::ready()
{
	if (connected)
	{
		if (!playing)
		{
			is_ready = !is_ready;

			sf::Packet ready_msg;
			ready_msg << cl_ready << id << is_ready;

			set_ack(ready_msg, sv_peel);

			if (is_ready)
				messages.add("You are ready", Message::Severity::LOW);
			else
				messages.add("You are not ready", Message::Severity::LOW);
		}
	}
	else
		messages.add("You are not connected to the server!", Message::Severity::HIGH);
}

void MultiplayerGame::process_packet(sf::Packet& packet)
{
	sf::Uint8 type;
	packet >> type;

	switch (type)
	{
		case sv_connect:
			messages.add("Connected...", Message::Severity::CRITICAL);
			connected = true;
			// TODO what to do with player count?
			break;
		case sv_disconnect:
		{
			sf::Uint8 reason;
			packet >> reason;

			string message {"Disconnected from server: "};
			switch (reason)
			{
				case 0:
					message.append("incompatible version");
					break;
				case 1:
					message.append("server is full");
					break;
				case 2:
					message.append("wrong password");
					break;
				case 3:
					message.append("game in progress");
					break;
				case 4:
					message.append("server shutting down");
					break;
				default:
					message.append("unknown reason");
			}

			connected = false;
			messages.add(message, Message::Severity::CRITICAL);
			break;
		}
		case sv_check:
		{
			string word;
			bool valid;
			packet >> word >> valid;

			dictionary[word] = valid;

			// TODO use one packet for word check to work better with ACK
			// if we still need the result for this word
			if (lookup_words.count(word))
			{
				if (!valid)
					bad_words[word] = lookup_words[word];

				lookup_words.erase(word);

				if (lookup_words.size() == 0)
					resolve_peel();
			}

			break;
		}
		case sv_peel:
		{
			sf::Int16 got_peel;
			sf::Int16 remaining;
			string peeler_id;
			string letters;

			packet >> got_peel >> remaining >> peeler_id >> letters;

			if (got_peel == peel_n + 1)
			{
				// first peel is special
				if (got_peel == 0)
				{
					playing = true;
					messages.clear();
					messages.add("SPLIT!", Message::Severity::HIGH);
				}

				++peel_n;
				if (peeler_id != id)
					// TODO get proper name and shit
					messages.add(peeler_id + ": PEEL!", Message::Severity::HIGH);

				if (remaining == 1)
					messages.add("1 letter remains", Message::Severity::LOW);
				else
				{
					std::stringstream rem;
					rem << remaining;
					messages.add(rem.str() + " letters remain", Message::Severity::LOW);
				}
			}
			else
			{
				cerr << "Peel out of order. Got " << (int)got_peel << ", expecting " << (int)(peel_n + 1);
				break;
			}

			cerr << "Received " << letters.size() << " letters: " << letters << endl;

			for (const auto& chr : letters)
				hand.add_tile(new Tile(chr));

			break;
		}
		case sv_dump:
		{
			string letters;
			packet >> letters;

			for (unsigned int i = 0; i < letters.size(); i++)
				hand.add_tile(new Tile(letters[i]));

			if (letters.size() == 1)
				messages.add("There are not enough tiles left to dump!", Message::Severity::HIGH);

			break;
		}
		case sv_done:
		{
			sf::Uint8 victory;
			packet >> victory;

			if (victory)
			{
				string winner_id;
				packet >> winner_id;

				if (winner_id == id)
					messages.add("You win!", Message::Severity::CRITICAL);
				else
					// TODO get proper name
					messages.add(winner_id + " has won the game!", Message::Severity::CRITICAL);
			}
			else
				messages.add("You win! All other players have resigned.", Message::Severity::CRITICAL);

			connected = false;
			playing = false;

			break;
		}
		default:
			cerr << "Received unknown packet type " << type << endl;
	}
}

void MultiplayerGame::dump()
{
	// TODO don't allow if still waiting for peel?
	Tile* dumped {grid.remove(cursor.get_pos())};
	if (dumped == nullptr)
	{
		messages.add("You need to select a tile to dump.", Message::Severity::LOW);
		return;
	}

	sf::Packet dump_request;
	dump_request << cl_dump << id << sf::Int8(dumped->ch());

	set_ack(dump_request, sv_dump);

	delete dumped;
}

// called once all words from peel have been looked up
bool MultiplayerGame::resolve_peel()
{
	if (bad_words.size() > 0)
	{
		cerr << "There are incorrect words" << endl;

		for (auto& word : bad_words)
		{
			messages.add(word.first + " is not a word.", Message::Severity::HIGH);
			// color incorrect tiles
			for (auto& pos: word.second)
				grid.bad_word(pos[0], pos[1], pos[2]);
		}

		return false;
	}


	sf::Int16 next_peel = peel_n + 1;

	cerr << "No incorrect words. Requesting peel " << (int)next_peel << endl;

	sf::Packet finished_peel;
	finished_peel << cl_peel << id << next_peel;

	set_ack(finished_peel, sv_peel);

	return true;
}

bool MultiplayerGame::peel()
{
	lookup_words.clear();
	bad_words.clear();

	if (!Game::peel())
		return false;

	const gridword_map& words {grid.get_words()};
	std::map<string, bool>::iterator it;

	// check words
	for (auto& word : words)
	{
		if ((it = dictionary.find(word.first)) == dictionary.end())
			lookup_words[word.first] = word.second;
		else if (!it->second)
			bad_words[word.first] = word.second;
	}

	if (lookup_words.size() > 0)
	{
		for (auto& word : lookup_words)
		{
			cerr << "Requesting lookup of " << word.first << endl;
			sf::Packet lookup;
			lookup << cl_check << id << word.first;
			socket.send(lookup, server_ip, server_port);
		}

		return false;
	}

	// all words are in local dictionary
	return resolve_peel();
}

// TODO need display for other players, letters remaining, etc.
void MultiplayerGame::draw_on(sf::RenderWindow& window, const sf::View& grid_view, const sf::View& gui_view) const
{
	Game::draw_on(window, grid_view, gui_view);
}

void MultiplayerGame::set_ack(sf::Packet& packet, sf::Uint8 response)
{
	if (ack != nullptr)
		delete ack;

	ack = new Acket(socket, server_ip, server_port, packet, response);
}

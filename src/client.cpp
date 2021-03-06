#include "client.hpp"

using std::cerr;
using std::endl;
using std::string;

Client::Client(const sf::View& view, const sf::Font& font, const sf::IpAddress& ip, unsigned short port, const std::string& name, bool _is_sp)
	: playing(false), hand(font), messages(font), server_ip(ip), server_port {port}, is_sp {_is_sp}
{
	std::mt19937 rng {std::random_device {}()};
	id = boost::uuids::to_string(boost::uuids::basic_random_generator<std::mt19937>(rng)());

	unsigned short client_port = server_port + 1;

	// find port to bind to
	while (socket.bind(client_port) != sf::Socket::Status::Done)
		++client_port;

	socket.setBlocking(false);

	set_pending(cl_connect);
	(*pending) << protocol_version << name;

	time_stale = 0.f; // SO FRESH
	poll_pause = 0.f; // haven't sent first packet yet
	timeout = 30.f; // long timeout for connection
	polling = 3.f; // and slow polling

	// TODO remove loading text when loading saved game
	if (is_sp)
		messages.add("Loading...", Message::Severity::CRITICAL);
	else
		messages.add("Connecting to " + server_ip.toString() + "...", Message::Severity::CRITICAL);

	send_pending();

	hand.set_view(view);
}

// construct single player game
Client::Client(const sf::View& view, const sf::Font& font)
	: Client(view, font, sf::IpAddress("127.0.0.1"), default_server_port, "singleplayer", true)
{
}

Client::~Client()
{
	grid.clear();

	hand.clear();

	clear_buffer();

	messages.clear();

	set_pending(cl_disconnect);
	send_pending();
	clear_pending();

	socket.unbind();
}

void Client::load(std::ifstream& save_file)
{
	// XXX stuff for server, intentionally unused here
	std::string dict_filename;
	uint8_t num;
	uint8_t den;
	std::getline(save_file, dict_filename, '\0');
	save_file.read(reinterpret_cast<char*>(&num), sizeof num);
	save_file.read(reinterpret_cast<char*>(&den), sizeof den);

	char ch;
	save_file.get(ch);

	// read hand
	while (ch != '\0')
	{
		uint8_t count;
		save_file.read(reinterpret_cast<char*>(&count), sizeof count);

		for (unsigned int i = 0; i < count; ++i)
			hand.add_tile(new Tile(ch));

		save_file.get(ch);
	}

	// read grid
	save_file.get(ch);
	while (!save_file.eof())
	{
		sf::Vector2i pos;
		save_file.read(reinterpret_cast<char*>(&pos.x), sizeof pos.x);
		save_file.read(reinterpret_cast<char*>(&pos.y), sizeof pos.y);

		grid.swap(pos, new Tile(ch));

		save_file.get(ch);
	}

	// reset file
	save_file.clear();
	save_file.seekg(0);

	// TODO this isn't necessarily true, we need more networking logic to
	// ensure that we have a server connection without a split
	// something like sv_connected
	playing = true;
	started = true;
}

void Client::clear_buffer()
{
	delete buffer;
	buffer = nullptr;
}

void Client::update_mouse_pos(const sf::RenderWindow& window, const sf::View& view, const sf::Vector2i& pos)
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
void Client::select()
{
	selecting = true;
	selected = false;
	sel1 = mcursor.get_pos();
}

// resize the current selection to the mouse cursor
void Client::resize_selection()
{
	sel2 = mcursor.get_pos();
	// update selection rect
	selection.set_size({(unsigned int)std::abs(sel1.x - sel2.x) + 1, (unsigned int)std::abs(sel1.y - sel2.y) + 1});
	selection.set_pos({std::min(sel1.x, sel2.x), std::min(sel1.y, sel2.y)});
}

// finish selection, move cursor (if necessary), return selection size
unsigned int Client::complete_selection()
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
void Client::quick_place()
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

void Client::cut()
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

void Client::flip_buffer()
{
	if (buffer != nullptr)
		buffer->transpose();
}

void Client::paste()
{
	if (buffer != nullptr)
	{
		buffer->paste(grid, hand);
		clear_buffer();
	}
	else
		messages.add("Cannot paste: no tiles were cut.", Message::Severity::LOW);
}

void Client::remove_at_mouse()
{
	// remove tile
	Tile* tile {grid.remove(mcursor.get_pos())};
	if (tile != nullptr)
		hand.add_tile(tile);
}

void Client::prompt_show()
{
	messages.add("Type a letter to highlight in play:", Message::Severity::HIGH);
}

void Client::show(char ch)
{
	messages.add("Highlighting " + string(1, ch) + "s.", Message::Severity::HIGH);
	if (!grid.highlight(ch))
		messages.add("There are no " + string(1, ch) + "s in play.", Message::Severity::HIGH);
}

// TODO it would be nice if we could do the bunch size check here, and not even ask for a letter
void Client::prompt_dump()
{
	messages.add("Type a letter to dump:", Message::Severity::HIGH);
}

void Client::remove()
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
		// if left of a tile
		else if (grid.get(pos + X) != nullptr)
			next = X;
		// if above a tile
		else if (grid.get(pos + Y) != nullptr)
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

void Client::place(char ch)
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

void Client::update_cursor(const sf::View& view)
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

void Client::set_zoom(float zoom)
{
	cursor.set_zoom(zoom);
	mcursor.set_zoom(zoom);
	selection.set_zoom(zoom);
}

// TODO need display for other players, letters remaining, etc.
void Client::draw_on(sf::RenderWindow& window, const sf::View& grid_view, const sf::View& gui_view) const
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

void Client::step(float time)
{
	grid.step(time);
	messages.step(time);

	// process incoming packets
	sf::Packet packet;
	sf::IpAddress ip;
	unsigned short port;
	if (socket.receive(packet, ip, port) == sf::Socket::Status::Done)
	{
		// TODO somehow verify that this is actually the server...
		process_packet(packet);
	}

	// process outgoing packets
	if (pending != nullptr)
	{
		time_stale += time;
		poll_pause += time;

		if (time_stale > timeout)
		{
			if (is_sp)
			{
				messages.add("Sorry, this is taking a while to load...", Message::Severity::CRITICAL);
				time_stale -= timeout; // TODO maybe disconnect with error if we've already joined? That shouldn't time out
			}
			else
			{
				messages.add("Disconnected from server: server timed out", Message::Severity::CRITICAL);
				disconnect();
			}
		}
		else if (poll_pause >= polling)
		{
			send_pending();
			poll_pause -= polling;
		}
	}
}

void Client::send_pending()
{
	if (pending == nullptr)
		cerr << "Attempt to send null packet!\n";
	else
		socket.send(*pending, server_ip, server_port);
}

void Client::set_pending(sf::Uint8 type)
{
	clear_pending(true);
	pending = new sf::Packet;
	(*pending) << type << id;
	pending_type = type;
}

void Client::clear_pending(bool force)
{
	if (pending == nullptr)
	{
		if (!force)
			cerr << "Attempt to clear null packet!\n";
	}
	else
	{
		delete pending;
		pending = nullptr;
	}

	pending_type = 255;

	time_stale = 0.f;
	poll_pause = 0.f;
}

void Client::ready()
{
	if (connected)
	{
		if (!playing)
		{
			// there's no point unreadying in SP
			if (is_sp && is_ready)
				return;

			is_ready = !is_ready;

			set_pending(cl_ready);
			(*pending) << is_ready;
			send_pending();

			if (is_sp)
				return;

			if (is_ready)
				messages.add("You are ready", Message::Severity::LOW);
			else
				messages.add("You are not ready", Message::Severity::LOW);
		}
	}
	else if (!is_sp)
		messages.add("You are not connected to the server!", Message::Severity::HIGH);
}

void Client::process_packet(sf::Packet& packet)
{
	sf::Uint8 type;
	packet >> type;

	switch (type)
	{
		case sv_info:
		{
			if (!connected)
			{
				if (!is_sp)
				{
					messages.add("Connected...", Message::Severity::CRITICAL);
					// TODO display ready reminder
				}

				connected = true;

				// want more responsiveness from server
				timeout = 5.f;
				polling = 0.5f;

				clear_pending();

				if (is_sp)
					ready();
			}

			string uuid;
			sf::Uint8 event;
			packet >> uuid >> event;

			if (uuid == id)
			{
				if (!playing) // we might be sending ready packets
					if ((is_ready && event == 2) || (!is_ready && event == 3)) // server acknowledged ready status
						clear_pending();
			}
			else
			{
				switch (event)
				{
					case 0: // joining
					{
						string name;
						packet >> name;

						sf::Int16 ack_n = ack_num;

						if (!players.count(uuid))
						{
							players[uuid] = Player(name);
							messages.add(name + " joined the game", Message::Severity::LOW);
							++ack_num;
						}
						else
						{
							cerr << "Redundant join for player: " << name << endl;
							--ack_n;
						}

						// acknowledge
						sf::Packet ack;
						ack << cl_ack << id << ack_n;
						socket.send(ack, server_ip, server_port);
						break;
					}
					case 1: // leaving
					{
						sf::Int16 ack_n = ack_num;

						if (players.count(uuid))
						{
							messages.add(players.at(uuid).get_name() + " left the game", Message::Severity::LOW);
							players.erase(uuid);
							++ack_num;
						}
						else
						{
							cerr << "Disconnect for unknown player: " << uuid << endl;
							--ack_n;
						}

						// acknowledge
						sf::Packet ack;
						ack << cl_ack << id << ack_n;
						socket.send(ack, server_ip, server_port);
						break;
					}
					case 2: // ready
					{
						if (players.count(uuid))
						{
							players.at(uuid).ready = true;
							messages.add(players.at(uuid).get_name() + " is ready to play", Message::Severity::LOW);
						}
						else
							cerr << "Ready for unknown player: " << uuid << endl;
						break;
					}
					case 3: // not ready
					{
						if (players.count(uuid))
						{
							players.at(uuid).ready = false;
							messages.add(players.at(uuid).get_name() + " is not ready", Message::Severity::LOW);
						}
						else
							cerr << "Unready for unknown player: " << uuid << endl;
						break;
					}
					default:
					{
						cerr << "Unknown sv_info event: " << (int)event << endl;
						break;
					}
				}
			}

			break;
		}
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

			disconnect();
			messages.add(message, Message::Severity::CRITICAL);
			break;
		}
		case sv_check:
		{
			string word;
			bool valid;
			packet >> word >> valid;

			cerr << word << " is " << (valid? "" : "not ") << "valid\n";

			dictionary[word] = valid;

			// if this is the word we requested
			if (pending_type == cl_check && lookup_words.size() > 0 && lookup_words.begin()->first == word)
			{
				if (!valid)
					bad_words[word] = lookup_words[word];

				lookup_words.erase(word);

				if (lookup_words.size() > 0)
				{
					// TODO DRY off
					gridword_map::iterator next_word = lookup_words.begin();
					cerr << "Requesting lookup of " << next_word->first << endl;
					set_pending(cl_check);
					(*pending) << next_word->first;

					send_pending();
				}
				else
				{
					clear_pending(true);
					resolve_peel();
				}
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

			// acknowledge
			sf::Int16 ack_n = ack_num;

			if (got_peel == peel_n + 1)
			{
				// if we're trying to peel, this means it worked
				if (pending_type == cl_peel)
				{
					clear_pending();
					waiting = false;
				}

				// first peel is special
				if (got_peel == 0)
				{
					if (!playing)
					{
						playing = true;
						messages.clear();
						messages.add("SPLIT!", Message::Severity::HIGH);

						started = true;
					}

					// reset ack count
					ack_n = 0;
					ack_num = 0;
				}
				// announce if someone else peeled
				else if (peeler_id != id)
					messages.add(players.at(peeler_id).get_name() + ": PEEL!", Message::Severity::HIGH);

				++peel_n;

				if (remaining < (int)players.size())
					messages.add("Final peel!", Message::Severity::HIGH);
				else
				{
					std::stringstream rem;
					rem << remaining;
					messages.add(rem.str() + " letters remain", Message::Severity::LOW);
				}

				cerr << "Received " << letters.size() << " letters: " << letters << endl;

				for (const auto& chr : letters)
					hand.add_tile(new Tile(chr));

				++ack_num;
			}
			else
			{
				cerr << "Peel out of order. Got " << (int)got_peel << ", expecting " << (int)(peel_n + 1);
				--ack_n;
			}

			sf::Packet ack;
			ack << cl_ack << id << ack_n;
			socket.send(ack, server_ip, server_port);

			break;
		}
		case sv_dump:
		{
			sf::Int16 dump_num;
			packet >> dump_num;

			// if we dumped and this is the right number
			if (pending_type == cl_dump)
			{
				if (dump_num == dump_n)
				{
					string letters;
					packet >> letters;

					for (unsigned int i = 0; i < letters.size(); i++)
						hand.add_tile(new Tile(letters[i]));

					if (letters.size() == 1)
						messages.add("There are not enough tiles left to dump!", Message::Severity::HIGH);

					clear_pending();
					waiting = false;
				}
				else
					cerr << "Got dump " << (int)(dump_num) << " but expected " << (int)dump_n << endl;
			}
			else
				cerr << "Received unexpected dump.\n";

			break;
		}
		case sv_done:
		{
			sf::Uint8 victory;
			packet >> victory;

			sf::Int16 ack_n = ack_num;

			if (playing)
			{
				if (victory)
				{
					string winner_id;
					packet >> winner_id;

					if (winner_id == id)
						messages.add("You win!", Message::Severity::CRITICAL);
					else
						messages.add(players.at(winner_id).get_name() + " has won the game!", Message::Severity::CRITICAL);
				}
				else
					messages.add("You win! All other players have resigned.", Message::Severity::CRITICAL);

				++ack_num;
			}
			else
				--ack_n;

			connected = false;
			playing = false;
			clear_pending(true);

			// acknowledge
			sf::Packet ack;
			ack << cl_ack << id << ack_n;
			socket.send(ack, server_ip, server_port);

			break;
		}
		default:
			cerr << "Received unknown packet type " << type << endl;
	}
}

void Client::dump(char ch)
{
	if (!connected)
	{
		if (!is_sp)
			messages.add("You are not connected to the server!", Message::Severity::HIGH);
		return;
	}

	if (!hand.has_any(ch))
	{
		messages.add("You don't have any " + string(1, ch) + "s in your hand!", Message::Severity::HIGH);
		return;
	}

	if (waiting)
	{
		// TODO autoretry
		messages.add("Waiting for server response. Try again in a moment.", Message::Severity::HIGH);
		return;
	}

	Tile* selected = hand.remove_tile(ch);
	set_pending(cl_dump);
	(*pending) << ++dump_n << sf::Int8(selected->ch());
	send_pending();

	waiting = true;

	delete selected;
}

// called once all words from peel have been looked up
bool Client::resolve_peel()
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

		waiting = false;

		return false;
	}


	sf::Int16 next_peel = peel_n + 1;

	cerr << "No incorrect words. Requesting peel " << (int)next_peel << endl;

	set_pending(cl_peel);
	(*pending) << next_peel;
	send_pending();

	waiting = true;

	return true;
}

bool Client::peel()
{
	if (!connected)
	{
		if (!is_sp)
			messages.add("You are not connected to the server!", Message::Severity::HIGH);
		return false;
	}

	lookup_words.clear();
	bad_words.clear();

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

	if (waiting)
	{
		// TODO auto retry?
		messages.add("Waiting for server response. Try again in a moment.", Message::Severity::HIGH);
		return false;
	}

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

	// all words are in local dictionary
	if (lookup_words.size() == 0)
		return resolve_peel();

	// need to request lookups
	gridword_map::iterator next_word = lookup_words.begin();
	cerr << "Requesting lookup of " << next_word->first << endl;
	set_pending(cl_check);
	(*pending) << next_word->first;

	send_pending();

	waiting = true;

	return false;
}

void Client::disconnect()
{
	connected = false;
	playing = false;
	clear_pending(true);
}

void Client::save(const std::string& filename, const Server& server) const
{
	std::ofstream save_file(filename);

	auto dict_filename = server.get_dict_filename();
	auto num = server.get_num();
	auto den = server.get_den();

	// save server parameters
	save_file.write(dict_filename.c_str(), dict_filename.length() + 1);
	save_file.write(reinterpret_cast<const char*>(&num), sizeof num);
	save_file.write(reinterpret_cast<const char*>(&den), sizeof den);

	// save hand
	for (char ch = 'A'; ch <= 'Z'; ++ch)
	{
		if (hand.has_any(ch))
		{
			save_file.put(ch);
			uint8_t count = hand.count(ch);
			save_file.write(reinterpret_cast<const char*>(&count), sizeof count);
		}
	}

	save_file.put('\0');

	// save grid
	for (Tile* tile : grid.internal())
	{
		if (tile != nullptr)
		{
			auto pos = tile->get_grid_pos();
			save_file.put(tile->ch());
			save_file.write(reinterpret_cast<const char*>(&pos.x), sizeof pos.x);
			save_file.write(reinterpret_cast<const char*>(&pos.y), sizeof pos.y);
		}
	}

	save_file.close();
}

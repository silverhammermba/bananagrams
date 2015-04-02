#include "game.hpp"

using std::string;

Game::Game(const std::string& dict_filename, uint8_t _bunch_num, uint8_t _bunch_den, unsigned int* counts, unsigned int _player_limit)
	: bunch_num(_bunch_num), bunch_den(_bunch_den), player_limit(_player_limit)
{
	std::ifstream words(dict_filename);
	if (!words.is_open())
	{
		// TODO indicate error somehow?
		finished = true;
		return;
	}

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

	if (bunch_den > 0)
		bunch = new FiniteBunch(bunch_num, bunch_den, counts);
	else
		bunch = new InfiniteBunch();
}

Game::~Game()
{
	delete bunch;
}

Player& Game::add_player(const string& id, const sf::IpAddress& ip, unsigned short port, const string& name)
{
	players[id] = Player(ip, port, name);
	return players[id];
}

void Game::remove_player(const string& id)
{
	for (auto chr : players.at(id).get_hand())
		bunch->add_tile(chr);

	players.erase(id);

	if (playing)
	{
		if (players.size() < 2)
		{
			ready_to_finish = true;
			finished = true;

			// either the last players wins by default or everyone left
			winner.clear();
		}
	}
	else
		try_to_start();
}

void Game::set_ready(const string& id, bool ready)
{
	if (!playing)
	{
		players.at(id).ready = ready;

		if (ready)
			try_to_start();
	}
}

// XXX assumes dump is valid!
string Game::dump(const string& id, const sf::Int16& dump_n, char chr)
{
	if (dump_n == players.at(id).get_dump() - 1)
		return players.at(id).last_dump();
	else if (dump_n == players.at(id).get_dump())
	{
		string letters;

		if (bunch->size() >= 3)
		{
			// take three
			for (unsigned int i = 0; i < 3; ++i)
				letters.append(1, bunch->get_tile());
			players.at(id).give_dump(chr, letters);

			bunch->add_tile(chr);
		}
		else
			letters.append(1, chr);

		return letters;
	}

	// XXX should never get here!!!
	return "";
}

// XXX assumes peeling is valid. returns true if game has ended
bool Game::peel()
{
	// if there aren't enough letters left
	if (bunch->size() < players.size())
	{
		ready_to_finish = true;
		finished = true;
		return true;
	}

	unsigned int num_letters = 1;
	ready_to_peel = false;

	// if this is the first peel
	if (peel_number++ == 0)
	{
		playing = true;

		unsigned int num_players = players.size();
		if (num_players * bunch_den <= 4 * bunch_num)
			num_letters = 21;
		else if (num_players * bunch_den <= 6 * bunch_num)
			num_letters = 15;
		else // num_players * bunch_den <= 8 * bunch_num
			num_letters = 11;

		// reset ack counters to track peels
		for (auto& pair : players)
			pair.second.reset_ack();
	}

	for (auto& pair : players)
	{
		string letters;
		for (unsigned int i = 0; i < num_letters; ++i)
			letters.append(1, bunch->get_tile());

		pair.second.give_peel(letters);
	}

	return false;
}

void Game::try_to_start()
{
	// if already started
	if (playing)
		return;

	// if not enough players
	if (player_limit > 1 && players.size() < 2)
		return;

	// if players aren't ready
	for (const auto& pair : players)
		if (!pair.second.ready)
			return;

	ready_to_peel = true;
}

void Game::check_waiting()
{
	for (const auto& pair : players)
		if (pair.second.has_pending())
			return;

	waiting = false;
}

#include "server.hpp"

using std::string;

Game::Game(unsigned int _bunch_num, unsigned int _bunch_den, unsigned int _player_limit)
	: player_limit(_player_limit)
{
	for (char ch = 'A'; ch <= 'Z'; ++ch)
		for (unsigned int i = 0; i < ((letter_count[ch - 'A'] * _bunch_num) / _bunch_den); ++i)
			random_insert(bunch, ch);

	auto it = bunch.begin();
	for (int i = 0; i < 20; ++i, ++it);
	bunch.erase(it, bunch.end());
}

Player& Game::add_player(const string& id, const sf::IpAddress& ip, unsigned short port, const string& name)
{
	players[id] = Player(ip, port, name);
	return players[id];
}

void Game::remove_player(const string& id)
{
	for (auto chr : players.at(id).get_hand())
		random_insert(bunch, chr);

	players.erase(id);

	if (playing)
	{
		if (players.size() < 2)
		{
			ready_to_finish = true;
			finished = true;

			// win by default
			if (players.size() > 0)
				winner = players.begin()->first;
			else
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

		if (bunch.size() >= 3)
		{
			// take three
			for (unsigned int i = 0; i < 3; ++i)
			{
				letters.append(1, bunch.back());
				bunch.pop_back();
			}
			players.at(id).give_dump(chr, letters);

			random_insert(bunch, chr);
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
	if (bunch.size() < players.size())
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
		num_letters = 10; // TODO calculate

		// reset ack counters to track peels
		for (auto& pair : players)
			pair.second.reset_ack();
	}

	for (auto& pair : players)
	{
		string letters;
		for (unsigned int i = 0; i < num_letters; ++i)
		{
			letters.append(1, bunch.back());
			bunch.pop_back();
		}

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
	if (players.size() < 2)
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

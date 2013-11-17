#include "server.hpp"

using std::string;

Game::Game(unsigned int _bunch_num, unsigned int _bunch_den, unsigned int _player_limit)
	: player_limit(_player_limit)
{
	for (char ch = 'A'; ch <= 'Z'; ++ch)
		for (unsigned int i = 0; i < ((letter_count[ch - 'A'] * _bunch_num) / _bunch_den); ++i)
			random_insert(bunch, ch);
}

void Game::add_player(const string& id, const sf::IpAddress& ip, const string& name)
{
	players[id] = Player(ip, name);
}

void Game::remove_player(const string& id)
{
	// TODO should randomly insert instead
	bunch.splice(bunch.end(), players.at(id).get_hand());

	players.erase(id);

	if (!playing)
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
			for (unsigned int i = 0; i < 3; i++)
			{
				letters.append(1, bunch.back());
				bunch.pop_back();
			}
			players.at(id).give_dump(letters);

			random_insert(bunch, (char)chr);
		}
		else
			letters.append(1, (char)chr);

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
		return true;

	unsigned int num_letters = 1;
	ready_to_peel = false;

	// if this is the first peel
	if (peel_number++ == 0)
	{
		playing = true;
		num_letters = 21; // TODO calculate

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
	if (players.size() < 1) // TODO for debugging only
		return;

	// if players aren't ready
	for (const auto& pair : players)
		if (!pair.second.ready)
			return;

	ready_to_peel = true;
}

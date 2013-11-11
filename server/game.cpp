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
	bunch.splice(bunch.end(), players[id].get_hand());

	players.erase(id);

	if (!playing)
		try_to_start();
}

void Game::set_ready(const string& id, bool ready)
{
	if (!playing)
	{
		players[id].ready = ready;

		if (ready)
			try_to_start();
	}
}

// XXX assumes dump is valid!
string Game::dump(const string& id, const sf::Int16& dump_n, char chr)
{
	if (dump_n == players[id].get_dump() - 1)
		return players[id].last_dump();
	else if (dump_n == players[id].get_dump())
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
			players[id].give_dump(letters);

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

	++peel_number;

	for (auto& pair : players)
	{
		pair.second.give_peel(bunch.back());
		bunch.pop_back();
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

	must_start = true;
}

void Game::start()
{
	must_start = false;
	playing = true;

	unsigned int num_letters = 21; // TODO calculate

	for (auto& pair : players)
	{
		string letters;
		for (unsigned int i = 0; i < num_letters; ++i)
		{
			letters.append(1, bunch.back());
			bunch.pop_back();
		}

		pair.second.give_split(letters);
	}
}

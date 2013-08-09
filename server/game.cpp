#include "server.hpp"

using std::cout;
using std::endl;
using std::string;

Game::Game(unsigned int _bunch_num, unsigned int _bunch_den, unsigned int _player_limit)
	: player_limit(_player_limit)
{
	for (char ch = 'A'; ch <= 'Z'; ++ch)
		for (unsigned int i = 0; i < ((letter_count[ch - 'A'] * _bunch_num) / _bunch_den); ++i)
			random_insert(bunch, ch);
}

bool Game::add_player(const string& id, Player& player)
{
	if (!playing && players.count(id) == 0)
	{
		players[id] = player;

		cout << "\n" << player.get_name() << " has joined the game";
		cout.flush();
	}
	else
		return false;
}

bool Game::remove_player(const string& id)
{
	if (players.count(id) > 0)
	{
		cout << "\n" << players[id].get_name() << " has left the game";
		cout.flush();
		players.erase(id);

		if (playing)
		{
			// TODO add player's letters back to bunch
		}
		else
			try_to_start();
	}
	else
		return false;
}

void Game::set_ready(const string& id, bool ready)
{
	if (!playing)
	{
		if (players.count(id) > 0)
		{
			players[id].ready = ready;

			if (ready)
				try_to_start();
		}
	}
}

string Game::dump(const string& id, char chr)
{
	string letters;

	if (playing && bunch.size() >= 3)
	{
		// take three
		for (unsigned int i = 0; i < 3; i++)
		{
			// TODO give letters to player
			letters.append(1, bunch.back());
			bunch.pop_back();
		}

		random_insert(bunch, (char)chr);
	}
	else
	{
		letters.append(1, (char)chr);

		if (playing)
			cout << "\n\tNot enough letters left for dump";
		else
			cout << "\n\tDump received before game started";

		cout.flush();
	}

	return letters;
}

// TODO fucking sloppy. three different conditions to report in one function
bool Game::peel(const string& id, sf::Int16 number, bool& success, bool& victory)
{

	// if game hasn't started or invalid player id
	if (!playing || players.count(id) == 0)
		return false;

	// if this is the correct peel number
	if (number == peel_number + 1)
	{
		remaining -= players.size();

		// if there aren't enough letters left
		if (remaining < 0)
		{
			winner = id;

			cout << "\n" << players[id].get_name() << " has won the game!";
			cout.flush();

			return true;
		}

		++peel_number;
		cout << "\n" << players[id].get_name() << " peeled (" << (int)peel_number << ")";
		cout.flush();

		return true;
	}
	else
	{
		cout << "\nPeel out of order: got " << (int)client_peel << ", expected " << (int)(peel_number + 1);
		cout.flush();

		return false;
	}
}

string Game::next_letter()
{
	string letter;
	letter.append(1, bunch.back());
	bunch.pop_back();

	return letter;
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

	cout << "\nStarting game";
	cout.flush();

	playing = true;

	// TODO determine number of letters based on bunch size/player number
	sf::Int16 remaining = bunch.size() - 21 * players.size();

	for (const auto& pair : players)
	{
		string letters;
		for (unsigned int i = 0; i < 21; i++)
		{
			letters.append(1, bunch.back());
			bunch.pop_back();
		}

		cout << "\n" << "Sending " << pair.second.get_name() << " " << letters;
		cout.flush();

		sf::Packet peel;
		peel << sv_peel << sf::Int16(peel_n) << remaining << pair.first << letters;
		socket.send(peel, pair.second.get_ip(), client_port);
	}
}

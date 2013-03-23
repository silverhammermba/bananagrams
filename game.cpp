#include "bananagrams.hpp"

using std::string;

Game::Game()
	: hand(font), messages(font)
{
	buffer = nullptr;
	clear_buffer();
	selected = false;
	selecting = false;
}

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

void Game::restart(const std::string& dict, int multiplier, int divider)
{
	end();

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

	// TODO validate somehow
	// TODO cache so we don't have to reload every time
	std::ifstream words(dict);
	if (!words.is_open())
	{
		std::cerr << "Couldn't find " << dict << "!\n";
		// TODO throw something?
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

	selected = false;
	selecting = false;
}

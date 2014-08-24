#include "client.hpp"

Bunch::Bunch() : rng(std::random_device()())
{
}

FiniteBunch::FiniteBunch(uint8_t num, uint8_t den, unsigned int* counts)
{
	for (char ch = 'A'; ch <= 'Z'; ++ch)
		for (unsigned int i = 0; i < ((letter_count[ch - 'A'] * num) / den) - (counts == nullptr ? 0 : counts[ch - 'A']); ++i)
			add_tile(new Tile(ch));
}

FiniteBunch::~FiniteBunch()
{
	for (auto tile : tiles)
		delete tile;
	tiles.clear();
}

unsigned int FiniteBunch::size() const
{
	return tiles.size();
}

void FiniteBunch::add_tile(Tile* tile)
{
	auto pos = std::uniform_int_distribution<>(0, tiles.size())(rng);

	auto it = tiles.begin();
	for (int i = 0; i != pos; ++i, ++it);
	tiles.insert(it, tile);
}

Tile* FiniteBunch::get_tile()
{
	if (tiles.size() == 0) return nullptr;
	Tile* tile = tiles.back();
	tiles.pop_back();
	return tile;
}

InfiniteBunch::InfiniteBunch()
	: dist(std::begin(letter_count), std::end(letter_count))
{
}

unsigned int InfiniteBunch::size() const
{
	return std::numeric_limits<unsigned int>::max();
}

void InfiniteBunch::add_tile(Tile* tile)
{
	delete tile;
}

Tile* InfiniteBunch::get_tile()
{
	return new Tile('A' + dist(rng));
}

#include "client.hpp"

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
	random_insert(tiles, tile);
}

Tile* FiniteBunch::get_tile()
{
	if (size == 0) return nullptr;
	Tile* tile = tiles.back();
	tiles.pop_back();
	return tile;
}

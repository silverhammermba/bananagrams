#include <SFML/Graphics.hpp>
#include "bananagrams.hpp"
#include "tile.hpp"
#include "grid.hpp"
#include "hand.hpp"
#include "buffer.hpp"

CutBuffer::CutBuffer(Grid& grid, int left, int top, int width, int height)
{
	sf::Vector2i min(left + width - 1, top + height - 1);
	sf::Vector2i max(left, top);

	// try to shrink selection
	for (int i = left; i < left + width; i++)
		for (int j = top; j < top + height; j++)
		{
			auto tile = grid.get(i, j);
			if (tile != nullptr)
			{
				if (i < min.x)
					min.x = i;
				if (i > max.x)
					max.x = i;
				if (j < min.y)
					min.y = j;
				if (j > max.y)
					max.y = j;
			}
		}

	// if nonempty
	if (min.x <= max.x && min.y <= max.y)
	{
		size = max - min + XY;
		pos = (max + min) / 2;

		for (int i = min.x; i <= max.x; i++)
			for (int j = min.y; j <= max.y; j++)
			{
				Tile* tile = grid.remove(i, j);
				if (tile != nullptr)
					tile->set_color(sf::Color(255, 255, 255, 100));
				tiles.push_back(tile);
			}
	}
}

CutBuffer::~CutBuffer()
{
	for (auto tile : tiles)
		if (tile != nullptr)
			delete tile;
}

void CutBuffer::transpose()
{
	std::vector<Tile*> temp(tiles.size(), nullptr);
	for (unsigned int i = 0; i < tiles.size(); i++)
		temp[(i % size.y) * size.x + i / size.y] = tiles[i];
	for (unsigned int i = 0; i < tiles.size(); i++)
		tiles[i] = temp[i];
	unsigned int tmp = size.x;
	size.x = size.y;
	size.y = tmp;

	set_pos(pos);
}

// put tiles back in grid, returning displaced tiles to hand
void CutBuffer::paste(Grid& grid, Hand& hand)
{
	for (int i = 0; i < size.x; i++)
		for (int j = 0; j < size.y; j++)
		{
			auto tile = tiles[i * size.y + j];
			if (tile != nullptr)
			{
				tile->set_color(sf::Color::White);

				Tile* r = grid.swap(sf::Vector2i(i, j) + pos - size / 2, tile);
				if (r != nullptr)
					hand.add_tile(r);
			}
		}
}

// return tiles to hand
void CutBuffer::clear(Hand& hand)
{
	for (auto tile : tiles)
		if (tile != nullptr)
			hand.add_tile(tile);
}

void CutBuffer::set_pos(const sf::Vector2i& p)
{
	pos = p;

	for (int i = 0; i < size.x; i++)
		for (int j = 0; j < size.y; j++)
		{
			auto tile = tiles[i * size.y + j];

			if (tile != nullptr)
				tile->set_grid_pos(sf::Vector2i(i, j) + pos - size / 2);
		}
}

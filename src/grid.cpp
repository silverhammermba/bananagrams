#include "client.hpp"

using std::array;
using std::string;
using std::vector;

Grid::~Grid()
{
	for (auto tile : grid)
		if (tile != nullptr)
			delete tile;
}

unsigned int Grid::convert(int x, int y) const
{
	if (x >= 0 && y >= 0)
		return 4 * bijection(x, y);
	if (x < 0 && y >= 0)
		return 4 * bijection(-x - 1, y) + 1;
	if (x >= 0 && y < 0)
		return 4 * bijection(x, -y - 1) + 2;
	else
		return 4 * bijection(-x - 1, -y - 1) + 3;
}

// for checking connectedness of grid
void Grid::traverse(int x, int y)
{
	Tile* tile {get(x, y)};
	if (tile == nullptr || tile->marked)
		return;

	tile->marked = true;
	traverse(x - 1, y);
	traverse(x + 1, y);
	traverse(x, y - 1);
	traverse(x, y + 1);
}

sf::Vector2f Grid::get_center() const
{
	return ((sf::Vector2f)(max + min) / (float)2.0 + sf::Vector2f(0.5, 0.5)) * (float)PPB;
}

// return the tile at the coords
Tile* Grid::get(int x, int y) const
{
	unsigned int n {convert(x, y)};
	if (n >= grid.size())
		return nullptr;
	return grid[n];
}

// remove the tile at the coords and return it
Tile* Grid::remove(int x, int y)
{
	unsigned int n {convert(x, y)};
	// if in bounds
	if (n < grid.size())
	{
		Tile* tile {grid[n]};

		// return if nothing was changed
		if (tile == nullptr)
			return nullptr;

		--tiles;

		// shrink grid, if possible
		grid[n] = nullptr;
		for (n = grid.size(); n > 0; --n)
			if (grid[n - 1] != nullptr)
				break;

		grid.resize(n, nullptr);

		if (tiles == 0)
		{
			// reset center if all tiles removed
			min = sf::Vector2i(0, 0);
			max = sf::Vector2i(0, 0);
		}
		// if tile is removed at a boundary
		else if (x == max.x || x == min.x || y == max.y || y == min.y)
		{
			// completely recalculate center :/
			bool first {true};
			for (auto tl : grid)
			{
				if (tl != nullptr)
				{
					const sf::Vector2i& pos {tl->get_grid_pos()};
					if (first)
						min = max = pos;
					else
					{
						if (pos.x > max.x)
							max.x = pos.x;
						else if (pos.x < min.x)
							min.x = pos.x;
						if (pos.y > max.y)
							max.y = pos.y;
						else if (pos.y < min.y)
							min.y = pos.y;
					}
					first = false;
				}
			}
		}

		// check for created words
		if (get(x + 1, y) != nullptr && get(x + 2, y) != nullptr)
			hwords[sf::Vector2i(x + 1, y)] = true;

		if (get(x, y + 1) != nullptr && get(x, y + 2) != nullptr)
			vwords[sf::Vector2i(x, y + 1)] = true;

		// check for destroyed words
		hwords.erase(sf::Vector2i(x, y));
		if (get(x - 2, y) == nullptr)
			hwords.erase(sf::Vector2i(x - 1, y));

		vwords.erase(sf::Vector2i(x, y));
		if (get(x, y - 2) == nullptr)
			vwords.erase(sf::Vector2i(x, y - 1));

		return tile;
	}
	return nullptr;
}

// exchange the tile at the coords for the given one, and return it
Tile* Grid::swap(int x, int y, Tile* tile)
{
	if (tile == nullptr)
		throw std::runtime_error("attempt to place NULL tile");
	unsigned int n {convert(x, y)};
	if (n >= grid.size())
		grid.resize(n + 1, nullptr);
	Tile* swp {grid[n]};
	tile->set_grid_pos(x, y);
	grid[n] = tile;

	if (swp != nullptr)
		return swp;

	if (tiles == 0)
		min = max = sf::Vector2i(x, y);
	else
	{
		if (x > max.x)
			max.x = x;
		else if (x < min.x)
			min.x = x;
		if (y > max.y)
			max.y = y;
		else if (y < min.y)
			min.y = y;
	}
	++tiles;

	last = sf::Vector2i(x, y);

	// check for created words
	if (get(x - 1, y) == nullptr)
	{
		if (get(x + 1, y) != nullptr)
			hwords[sf::Vector2i(x, y)] = true;
	}
	else if (get(x - 2, y) == nullptr)
		hwords[sf::Vector2i(x - 1, y)] = true;

	if (get(x, y - 1) == nullptr)
	{
		if (get(x, y + 1) != nullptr)
			vwords[sf::Vector2i(x, y)] = true;
	}
	else if (get(x, y - 2) == nullptr)
		vwords[sf::Vector2i(x, y - 1)] = true;

	// check for destroyed words
	if (get(x + 1, y) != nullptr)
		hwords.erase(sf::Vector2i(x + 1, y));

	if (get(x, y + 1) != nullptr)
		vwords.erase(sf::Vector2i(x, y + 1));

	return nullptr;
}

void Grid::clear()
{
	for (auto tile : grid)
		if (tile != nullptr)
			delete tile;
	grid.clear();
	tiles = 0;
	min = {0, 0};
	max = {0, 0};
	defined.clear();
	hwords.clear();
	vwords.clear();
}

// animate tiles
void Grid::step(float time)
{
	for (auto tile: grid)
	{
		if (tile != nullptr)
		{
			auto color = tile->get_color();
			tile->set_color(color + sf::Color(time * 300, time * 300, time * 300));
		}
	}
}

// test if grid has at least one word and is continuous
bool Grid::is_continuous()
{
	// need at least one word to be valid (also we should never get here if grid is empty)
	if (hwords.size() == 0 && vwords.size() == 0)
		return false;

	auto start = hwords.begin();
	if (hwords.size() == 0)
		start = vwords.begin();

	// grid must be continuous
	for (auto tile: grid)
		if (tile != nullptr)
			tile->marked = false;

	// starting points guaranteed to be non-null
	traverse(start->first.x, start->first.y);

	bool valid {true};
	for (auto tile: grid)
	{
		if (tile != nullptr && !tile->marked)
		{
			valid = false;
			tile->set_color(sf::Color(255, 50, 50));
		}
	}

	return valid;
}

gridword_map& Grid::get_words()
{
	std::stringstream temp;
	vector<string> defns;
	Tile* tile;
	words.clear();

	// get words
	bool define; // whether we should define this word
	for (auto& pair: hwords)
	{
		temp.str("");
		define = false;
		for (int x = pair.first.x; (tile = get(x, pair.first.y)) != nullptr; x++)
		{
			temp << tile->ch();
			if (x == last.x && pair.first.y == last.y)
				define = true;
		}
		if (define)
			defns.push_back(temp.str());
		if (!words.count(temp.str()))
			words[temp.str()] = vector<array<int, 3>>();
		words[temp.str()].push_back(array<int, 3>{{pair.first.x, pair.first.y, 0}});
	}
	for (auto& pair: vwords)
	{
		temp.str("");
		define = false;
		for (int y = pair.first.y; (tile = get(pair.first.x, y)) != nullptr; y++)
		{
			temp << tile->ch();
			if (pair.first.x == last.x && y == last.y)
				define = true;
		}
		if (define)
			defns.push_back(temp.str());
		if (!words.count(temp.str()))
			words[temp.str()] = vector<array<int, 3>>();
		words[temp.str()].push_back(array<int, 3>{{pair.first.x, pair.first.y, 1}});
	}

	return words;
}

bool Grid::highlight(char ch)
{
	bool found = false;

	for (auto tile : grid)
	{
		if (tile && tile->ch() == ch)
		{
			tile->set_color(sf::Color(50, 50, 255));
			found = true;
		}
	}

	return found;
}

void Grid::bad_word(int x, int y, int dir)
{
	// TODO with multiplayer games, the word might no longer exist!!!
	Tile* tile;
	int coord[2];
	for (coord[0] = x, coord[1] = y; (tile = get(coord[0], coord[1])) != nullptr; coord[dir]++)
		tile->set_color(sf::Color(255, 50, 50));
}

void Grid::draw_on(sf::RenderWindow& window) const
{
	for (auto tile: grid)
		if (tile != nullptr)
			tile->draw_on(window);
}

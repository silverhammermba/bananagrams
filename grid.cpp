#include "bananagrams.hpp"

using std::array;
using std::string;
using std::vector;

Grid::Grid() : grid(), min(0, 0), max(0, 0)
{
	tiles = 0;
}

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
	Tile* tile = get(x, y);
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
	unsigned int n = convert(x, y);
	if (n >= grid.size())
		return nullptr;
	return grid[n];
}

// remove the tile at the coords and return it
Tile* Grid::remove(int x, int y)
{
	unsigned int n = convert(x, y);
	// if in bounds
	if (n < grid.size())
	{
		Tile* tile = grid[n];

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
			bool first = true;
			for (auto tl : grid)
			{
				if (tl != nullptr)
				{
					const sf::Vector2i& pos = tl->get_grid_pos();
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
	unsigned int n = convert(x, y);
	if (n >= grid.size())
		grid.resize(n + 1, nullptr);
	Tile* swp = grid[n];
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

// check for connectedness and valid words
bool Grid::is_valid(vector<string>& messages)
{
	// need at least one word to be valid
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

	bool valid = true;
	for (auto tile: grid)
		if (tile != nullptr && !tile->marked)
		{
			valid = false;
			tile->set_color(sf::Color(255, 50, 50));
		}

	if (!valid)
	{
		messages.push_back("Your tiles are not all connected.");
		return false;
	}

	std::stringstream temp;
	std::map<string, vector<array<int, 3>>> words;
	vector<string> defns;
	Tile* tile;

	// get words
	for (auto& pair: hwords)
	{
		temp.str("");
		for (unsigned int x = pair.first.x; (tile = get(x, pair.first.y)) != nullptr; x++)
			temp << tile->ch();
		if (!words.count(temp.str()))
				words[temp.str()] = vector<array<int, 3>>();
		words[temp.str()].push_back(array<int, 3>{{pair.first.x, pair.first.y, 0}});
	}
	for (auto& pair: vwords)
	{
		temp.str("");
		for (unsigned int y = pair.first.y; (tile = get(pair.first.x, y)) != nullptr; y++)
			temp << tile->ch();
		if (!words.count(temp.str()))
				words[temp.str()] = vector<array<int, 3>>();
		words[temp.str()].push_back(array<int, 3>{{pair.first.x, pair.first.y, 1}});
	}

	// TODO only define the most recently spelled words
	// check words
	for (auto& word : words)
	{
		auto it = dictionary.find(word.first);

		// if invalid
		if (it == dictionary.end())
		{
			// if this is first error, clear definitions
			if (valid)
				messages.clear();
			valid = false;
			messages.push_back(word.first + " is not a word.");
			int coord[2];
			// color incorrect tiles
			for (auto& pos: word.second)
				for (coord[0] = pos[0], coord[1] = pos[1]; (tile = get(coord[0], coord[1])) != nullptr; coord[pos[2]]++)
					tile->set_color(sf::Color(255, 50, 50));
		}
		// if valid and defined
		else if (valid && std::rand() % 100 == 0 && it->second.length() > 0)
		{
			// check if we have already displayed the definition
			bool defd = false;
			for (string& wd : defined)
				if (word.first == wd)
				{
					defd = true;
					break;
				}
			if (!defd)
				for (string& wd : defns)
					if (word.first == wd)
					{
						defd = true;
						break;
					}

			if (!defd)
			{
				defns.push_back(word.first);
				messages.push_back(word.first + ": " + it->second);
			}
		}
	}

	// if error-free, keep track of displayed definitions
	if (valid)
		for (string& wd : defns)
			defined.push_back(wd);

	return valid;
}

void Grid::draw_on(sf::RenderWindow& window) const
{
	for (auto tile: grid)
		if (tile != nullptr)
			tile->draw_on(window);
}

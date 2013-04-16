#include "bananagrams.hpp"

Hand::Hand(const sf::Font& font)
{
	// prepare counts numbers
	for (char ch = 'A'; ch <= 'Z'; ch++)
	{
		number[ch - 'A'].setFont(font);
		number[ch - 'A'].setCharacterSize(PPB / 4.0);
		number[ch - 'A'].setColor(sf::Color::Black);
	}
}

Hand::~Hand()
{
	clear();
}

// position tiles in list in nice rows
void Hand::position_list(std::list<Tile*>& l)
{
	if (l.size() == 0)
		return;
	auto size = gui_view.getSize();
	float padding {PPB / 8.f};
	float min_width {PPB + padding};
	// leave PPB/2 space on either side of tiles, one tile gets full PPB width
	unsigned int max_per_row {(unsigned int)((size.x - (padding * 2 + PPB)) / (int)min_width + 1)};
	unsigned int rows {(unsigned int)l.size() / max_per_row + (l.size() > (l.size() / max_per_row) * max_per_row ? 1 : 0)};
	auto tile = l.begin();
	for (unsigned int i = 0; tile != l.end(); i++, tile++)
	{
		// number of tiles in this row
		unsigned int row_size {i >= (l.size() / max_per_row) * max_per_row ? (unsigned int)l.size() % max_per_row : max_per_row};
		float room_per_tile {row_size == 1 ? 0 : (size.x - PPB - padding * 2) / (row_size - 1)};
		// maximum PPB/4 spacing between tiles
		if (room_per_tile > (PPB * 5) / 4.0)
			room_per_tile = (PPB * 5) / 4.0;
		// this should be <= size.x - PPB
		float room_required {room_per_tile * (row_size - 1) + PPB};
		(*tile)->set_pos((i % max_per_row) * room_per_tile + (size.x - room_required) / 2.f, size.y - (rows - (i / max_per_row)) * (PPB + padding));
	}
}

void Hand::counts(sf::RenderWindow& window)
{
	position_list(single);

	for (auto tile: single)
	{
		tile->draw_on(window);
		number[tile->ch() - 'A'].setPosition(tile->get_pos() + sf::Vector2f(PPB / 32.0, 0));
		window.draw(number[tile->ch() - 'A']);
	}
}

void Hand::stacks(sf::RenderWindow& window)
{
	auto size = gui_view.getSize();
	unsigned int nonempty {0};
	for (char ch = 'A'; ch <= 'Z'; ch++)
		if (has_any(ch))
			nonempty++;
	if (nonempty == 0)
		return;
	float padding {PPB / 8.f};
	float room_per_tile {nonempty == 1 ? 0 : (size.x - PPB - padding * 2) / (float)(nonempty - 1)};
	if (room_per_tile > (PPB * 5) / 3.0)
		room_per_tile = (PPB * 5) / 3.0;
	// center tiles
	float x {(size.x - room_per_tile * (nonempty - 1) - PPB) / 2.0f};
	for (char ch = 'A'; ch <= 'Z'; ch++)
	{
		if (has_any(ch))
		{
			unsigned int i {0};
			for (auto tile: tiles[ch - 'A'])
			{
				tile->set_pos(x + (i * PPB) / 16.f, size.y - PPB - padding - (i * PPB) / 16.f);
				i++;
			}
			x += room_per_tile;
		}
	}

	for (char ch = 'Z'; ch >= 'A'; --ch)
		for (auto tile: tiles[ch - 'A'])
			tile->draw_on(window);
}

void Hand::ordered(sf::RenderWindow& window)
{
	position_list(sort);

	for (auto tile: sort)
		tile->draw_on(window);
}

void Hand::scrambled(sf::RenderWindow& window)
{
	position_list(scram);

	for (auto tile: scram)
		tile->draw_on(window);
}

void Hand::reshuffle()
{
	scram.clear();
	for (char ch = 'A'; ch <= 'Z'; ch++)
	{
		for (auto tile: tiles[ch - 'A'])
			random_insert(scram, tile);
	}
}

void Hand::clear()
{
	for (char ch = 'A'; ch <= 'Z'; ch++)
	{
		for (auto tile : tiles[ch - 'A'])
			delete tile;
		tiles[ch - 'A'].clear();
	}
	scram.clear();
	sort.clear();
	single.clear();
}

void Hand::add_tile(Tile* tile)
{
	tiles[tile->ch() - 'A'].push_back(tile);

	tile->set_color(sf::Color::White);

	// update persistent structures
	scram.push_back(tile);
	if (tiles[tile->ch() - 'A'].size() == 1)
	{
		for (auto it = single.begin(); ; it++)
			if (it == single.end() || (*it)->ch() > tile->ch())
			{
				single.insert(it, tiles[tile->ch() - 'A'][0]);
				break;
			}
	}
	std::stringstream str;
	str << tiles[tile->ch() - 'A'].size();
	number[tile->ch() - 'A'].setString(str.str());
	for (auto it = sort.begin(); ; it++)
		if (it == sort.end() || (*it)->ch() >= tile->ch())
		{
			sort.insert(it, tile);
			break;
		}
}

Tile* Hand::remove_tile(char ch)
{
	Tile* tile {nullptr};

	if (has_any(ch))
	{
		tile = tiles[ch - 'A'].back();
		tiles[ch - 'A'].pop_back();

		// update persistent structures
		scram.remove(tile);
		sort.remove(tile);
		if (has_any(tile->ch()))
		{
			std::stringstream str;
			str << tiles[tile->ch() - 'A'].size();
			number[tile->ch() - 'A'].setString(str.str());
		}
		else
			single.remove(tile);
	}

	return tile;
}

void Hand::set_scrambled()
{
	if (draw_func == &Hand::scrambled)
		reshuffle();
	else
		draw_func = &Hand::scrambled;
}

void Hand::set_sorted()
{
	draw_func = &Hand::ordered;
}

void Hand::set_counts()
{
	draw_func = &Hand::counts;
}

void Hand::set_stacked()
{
	draw_func = &Hand::stacks;
}

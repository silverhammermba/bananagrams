#ifndef HAND_HPP
#define HAND_HPP

#include <algorithm>
#include <list>
#include <sstream>

#include <SFML/Graphics.hpp>

#include "constants.hpp"
#include "tile.hpp"

// TODO inefficient
class Hand
{
	std::vector<Tile*> tiles[26];
	std::list<Tile*> scram; // for shuffle
	std::list<Tile*> sort; // for ordered
	std::list<Tile*> single; // for counts
	sf::Text number[26];

	sf::View gui_view;

	// position tiles in std::list in nice rows
	void position_list(std::list<Tile*>& l);

	// tile drawing functions
	void counts(sf::RenderWindow& window) const;
	void stacks(sf::RenderWindow& window) const;
	void ordered(sf::RenderWindow& window) const;
	void scrambled(sf::RenderWindow& window) const;

	void (Hand::*draw_func)(sf::RenderWindow&) const {&Hand::scrambled};

	void reshuffle();

public:
	Hand(const sf::Font& font);
	~Hand();

	inline unsigned int count(char ch) const
	{
		return tiles[ch - 'A'].size();
	}

	inline bool has_any(char ch) const
	{
		return count(ch) > 0;
	}

	void set_view(const sf::View& view);

	bool is_empty() const;

	void clear();

	void add_tile(Tile* tile);
	Tile* remove_tile(char ch);

	// position whatever the current arrangement is
	void position_tiles();

	inline void draw_on(sf::RenderWindow& window) const
	{
		(this->*draw_func)(window);
	}

	void set_scrambled();
	void set_sorted();
	void set_counts();
	void set_stacked();
};

#endif

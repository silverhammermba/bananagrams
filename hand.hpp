// TODO inefficient
class Hand
{
	std::vector<Tile*> tiles[26];
	std::list<Tile*> scram; // for shuffle
	std::list<Tile*> sort; // for ordered
	std::list<Tile*> single; // for counts
	sf::Text number[26];

	// position tiles in std::list in nice rows
	void position_list(std::list<Tile*>& l);
	// tile arrangement functions
	void counts(sf::RenderWindow& window);
	void stacks(sf::RenderWindow& window);
	void ordered(sf::RenderWindow& window);
	void scrambled(sf::RenderWindow& window);

	void (Hand::*draw_func)(sf::RenderWindow&) {&Hand::scrambled};

	void reshuffle();
public:
	Hand(const sf::Font& font);
	~Hand();

	inline bool has_any(char ch) const
	{
		return tiles[ch - 'A'].size() > 0;
	}

	void clear();

	void add_tile(Tile* tile);
	Tile* remove_tile(char ch);

	inline void draw_on(sf::RenderWindow& window)
	{
		(this->*draw_func)(window);
	}

	void set_scrambled();
	void set_sorted();
	void set_counts();
	void set_stacked();
};

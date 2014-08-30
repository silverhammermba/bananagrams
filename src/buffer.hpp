#ifndef BUFFER_HPP
#define BUFFER_HPP

class CutBuffer
{
	sf::Vector2i pos;
	sf::Vector2i size;
	std::vector<Tile*> tiles;
public:
	CutBuffer(Grid& grid, int left, int top, const sf::Vector2u& size);
	~CutBuffer();

	inline bool is_empty() const
	{
		return tiles.size() == 0;
	}

	void transpose();

	// put tiles back in grid, returning displaced tiles to hand
	void paste(Grid& grid, Hand& hand);
	// return tiles to hand
	void clear(Hand& hand);

	// set center to p
	void set_pos(const sf::Vector2i& p);

	void draw_on(sf::RenderWindow & window) const;
};

#endif

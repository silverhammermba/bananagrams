#ifndef TILE_HPP
#define TILE_HPP

class Tile
{
	char character;
	sf::Sprite sprite;
	sf::Vector2i gpos {0, 0}; // position on grid (not always meaningful)
public:
	bool marked; // for checking grid connectedness

	Tile(char ch)
		: character {ch}, sprite {tile_texture[ch - 'A'].getTexture()}
	{
	}

	// get character
	inline char ch() const
	{
		return character;
	}

	inline void set_pos(float x, float y)
	{
		sprite.setPosition(x, y);
	}

	inline void set_grid_pos(int x, int y)
	{
		gpos.x = x;
		gpos.y = y;
		sprite.setPosition(x * (int)PPB, y * (int)PPB);
	}

	inline void set_grid_pos(const sf::Vector2i& pos)
	{
		gpos = pos;
		sprite.setPosition(pos.x * (int)PPB, pos.y * (int)PPB);
	}

	inline const sf::Color& get_color() const
	{
		return sprite.getColor();
	}

	inline void set_color(const sf::Color& color)
	{
		sprite.setColor(color);
	}

	inline const sf::Vector2f& get_pos() const
	{
		return sprite.getPosition();
	}

	inline const sf::Vector2i& get_grid_pos() const
	{
		return gpos;
	}

	inline void draw_on(sf::RenderWindow & window) const
	{
		window.draw(sprite);
	}
};

#endif

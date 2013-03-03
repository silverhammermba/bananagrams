class Tile
{
	char character;
	sf::Sprite sprite;
public:
	bool marked; // for checking grid connectedness

	Tile(char ch);

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
		sprite.setPosition(x * (int)PPB, y * (int)PPB);
	}

	inline void set_grid_pos(const sf::Vector2i& pos)
	{
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

	inline void draw_on(sf::RenderWindow & window) const
	{
		window.draw(sprite);
	}
};

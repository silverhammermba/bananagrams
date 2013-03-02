#ifndef CURSOR_HPP
#define CURSOR_HPP

class Cursor
{
	sf::Vector2i pos;
	float outline_thickness;
	sf::RectangleShape cursor;
public:
	Cursor(float thick, sf::Color fill, sf::Color outline);

	inline const sf::Vector2i& get_pos() const
	{
		return pos;
	}

	inline sf::Vector2f get_center() const
	{
		return ((sf::Vector2f)pos + sf::Vector2f(0.5, 0.5)) * (float)PPB;
	}

	void set_pos(const sf::Vector2i& p);

	inline void move(const sf::Vector2i& d)
	{
		set_pos(pos + d);
	}

	inline void draw_on(sf::RenderWindow& window) const
	{
		window.draw(cursor);
	}
};

#endif

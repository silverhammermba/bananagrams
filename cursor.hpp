class Cursor
{
	sf::Vector2u size;
	sf::Vector2i pos;
	float outline_thickness;
	sf::RectangleShape cursor;
public:
	Cursor(const sf::Vector2u& sz, float thick, sf::Color fill, sf::Color outline);

	inline const sf::Vector2i& get_pos() const
	{
		return pos;
	}

	void set_pos(const sf::Vector2i& p);

	inline const sf::Vector2u& get_size() const
	{
		return size;
	}

	void set_size(const sf::Vector2u& sz);

	inline sf::Vector2f get_center() const
	{
		return ((sf::Vector2f)pos + sf::Vector2f(0.5, 0.5)) * (float)PPB;
	}

	inline void move(const sf::Vector2i& d)
	{
		set_pos(pos + d);
	}

	void set_zoom(float zoom);

	inline void draw_on(sf::RenderWindow& window) const
	{
		window.draw(cursor);
	}
};

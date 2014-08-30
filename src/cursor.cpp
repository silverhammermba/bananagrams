#include "cursor.hpp"

Cursor::Cursor(const sf::Vector2u& sz, float thick, sf::Color fill, sf::Color outline)
	: size {sz}, cursor {sf::Vector2f(PPB * sz.x - thick * 2, PPB * sz.y - thick * 2)}
{
	set_pos(sf::Vector2i {0, 0});
	cursor.setOutlineThickness(outline_thickness = thick);

	cursor.setFillColor(fill);
	cursor.setOutlineColor(outline);
}

void Cursor::set_size(const sf::Vector2u& sz)
{
	size = sz;

	cursor.setSize((sf::Vector2f)(size * PPB) - (sf::Vector2f)XY * outline_thickness);
}

void Cursor::set_pos(const sf::Vector2i& p)
{
	pos = p;

	cursor.setPosition((sf::Vector2f)(pos * (int)PPB) + (sf::Vector2f)XY * (float)(outline_thickness * 0.5));
}

void Cursor::set_zoom(float zoom)
{
	cursor.setOutlineThickness(outline_thickness * zoom);
	set_size(size);
	set_pos(pos);
}

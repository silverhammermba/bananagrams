#include <SFML/Graphics.hpp>
#include "bananagrams.hpp"
#include "cursor.hpp"

Cursor::Cursor(float thick, sf::Color fill, sf::Color outline)
	: pos(0, 0), cursor(sf::Vector2f(PPB - thick * 2, PPB - thick * 2))
{
	cursor.setOutlineThickness(outline_thickness = thick);

	cursor.setFillColor(fill);
	cursor.setOutlineColor(outline);
}

void Cursor::set_pos(const sf::Vector2i& p)
{
	pos = p;

	cursor.setPosition((sf::Vector2f)(pos * PPB) + (sf::Vector2f)XY * cursor.getOutlineThickness());
}

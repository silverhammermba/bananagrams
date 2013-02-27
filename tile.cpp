#include <SFML/Graphics.hpp>
#include "bananagrams.hpp"
#include "tile.hpp"

Tile::Tile(char ch) : sprite(tile_texture[ch - 'A'].getTexture())
{
	character = ch;
}

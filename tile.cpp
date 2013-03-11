#include "bananagrams.hpp"

Tile::Tile(char ch) : sprite(tile_texture[ch - 'A'].getTexture()), gpos(0, 0)
{
	character = ch;
}

#include "bananagrams.hpp"

Tile::Tile(char ch) : sprite(tile_texture[ch - 'A'].getTexture())
{
	character = ch;
}

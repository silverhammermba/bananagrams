#include "client.hpp"

Tile::Tile(char ch)
	: character {ch}, sprite {tile_texture[ch - 'A'].getTexture()}
{
}

#include "tile.hpp"

sf::RenderTexture Tile::texture[26];

Tile::Tile(char ch)
	: character {ch}, sprite {texture[ch - 'A'].getTexture()}
{
}

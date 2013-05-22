#include "server.hpp"

Player::Player()
	: peel {0}
{
}

sf::Packet& operator >>(sf::Packet& packet, Player& player)
{
	packet >> player.name;
	return packet;
}

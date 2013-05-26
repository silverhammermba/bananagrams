#include "server.hpp"

Player::Player(const sf::IpAddress& _ip)
	: ip {_ip}, peel {0}
{
}

sf::Packet& operator >>(sf::Packet& packet, Player& player)
{
	packet >> player.name;
	return packet;
}

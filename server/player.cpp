#include "server.hpp"

Player::Player(const sf::IpAddress& _ip)
	: ip {_ip}
{
}

sf::Packet& operator >>(sf::Packet& packet, Player& player)
{
	packet >> player.name;
	return packet;
}

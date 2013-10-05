#include "server.hpp"

Player::Player(const sf::IpAddress& _ip, const std::string& _name)
	: ip {_ip}, name {_name}
{
}

void Player::add_dump(const std::string& letters)
{
	dumps.push_back(letters);
	// TODO add letters to hand as well
}

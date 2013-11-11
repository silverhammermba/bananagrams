#include "server.hpp"

Player::Player(const sf::IpAddress& _ip, const std::string& _name)
	: ip {_ip}, name {_name}, dump_letters {""}
{
}

std::string Player::get_hand_str() const
{
	std::string str;
	for (const auto& letter : hand)
		str.append(1, letter);
	return str;
}

void Player::give_dump(const std::string& letters)
{
	// TODO remove dumped letter
	for (auto& letter : letters)
		hand.push_back(letter);
	dump_letters = letters;
	++dump_n;
}

void Player::give_split(const std::string& letters)
{
	for (auto& letter : letters)
		hand.push_back(letter);
}

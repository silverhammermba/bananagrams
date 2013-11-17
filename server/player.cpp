#include "server.hpp"

Player::Player(const sf::IpAddress& _ip, const std::string& _name)
	: ip {_ip}, name {_name}, dump_letters {""}
{
}

Player::~Player()
{
	for (auto packet : pending)
		delete packet;
}

void Player::give_dump(char chr, const std::string& letters)
{
	// remove dumped letter
	for (auto it = hand.begin(); it != hand.end(); ++it)
		if (*it == chr)
		{
			hand.erase(it);
			break;
		}

	// add new ones
	for (auto& letter : letters)
		hand.push_back(letter);

	dump_letters = letters;
	++dump_n;
}

void Player::give_peel(const std::string& letters)
{
	peel = letters;
	for (auto& letter : letters)
		hand.push_back(letter);
}

void Player::add_pending(const sf::Packet& packet)
{
	// if this the first packet, reset timers
	if (pending.size() == 0)
	{
		timeout = 0;
		poll = 0;
	}
	pending.push_back(new sf::Packet(packet));
}

bool Player::acknowledged(const sf::Int16& ack_num)
{
	if (pending.empty())
		return false;

	if (ack_num != ack_count)
		return false;

	++ack_count;
	delete pending.front();
	pending.pop_front();

	return true;
}

#ifndef PLAYER_HPP
#define PLAYER_HPP

class Player
{
	sf::IpAddress ip;
	unsigned short port;
	std::string name;
	std::string dump_letters;
	std::string peel;
	sf::Int16 dump_n {0};
	std::list<char> hand;

	float timeout;
	sf::Int16 ack_count {0};
	std::list<sf::Packet*> pending;
public:
	bool ready {false};
	float poll;

	explicit Player() {}; // XXX to appease the players map
	Player(const std::string& _name);
	Player(const sf::IpAddress& _ip, unsigned short _port, const std::string& _name);
	~Player();

	inline const sf::IpAddress& get_ip() const
	{
		return ip;
	}

	inline unsigned short get_port() const
	{
		return port;
	}

	inline const std::string& get_name() const
	{
		return name;
	}

	inline const std::string& last_dump() const
	{
		return dump_letters;
	}

	inline const sf::Int16& get_dump() const
	{
		return dump_n;
	}

	inline std::list<char>& get_hand()
	{
		return hand;
	}

	inline const std::string& get_peel() const
	{
		return peel;
	}

	inline float get_timeout() const
	{
		return timeout;
	}

	inline float get_poll() const
	{
		return poll;
	}

	void give_dump(char chr, const std::string& letters);
	void give_peel(const std::string& letters);

	void add_pending(const sf::Packet& packet);

	inline bool has_pending() const
	{
		return !pending.empty();
	}

	inline const sf::Packet& get_pending() const
	{
		return *pending.front();
	}

	void step(float elapsed)
	{
		timeout += elapsed;
		poll += elapsed;
	}

	bool acknowledged(const sf::Int16& ack_num);

	inline void reset_ack()
	{
		ack_count = 0;
		// pending should already be empty
	}
};

#endif

// class for resending packets until a proper acknowledgement is received
class Acket
{
	sf::UdpSocket& socket;
	const sf::IpAddress& ip;
	unsigned short port;
	sf::Packet packet;
	sf::Uint8 response;
	float time {0};
	float resend {0};

	void send()
	{
		socket.send(packet, ip, port);
	}
public:
	Acket(sf::UdpSocket& _socket, const sf::IpAddress& _ip, unsigned short _port, sf::Packet& _packet, sf::Uint8 _response)
		: socket (_socket), ip (_ip), port {_port}, packet {_packet}, response {_response}
	{
	}

	void age(float step)
	{
		time += step;
		resend += step;

		if (resend >= 1.0)
			send();

		resend -= 1.0;
	}

	inline bool is_timed_out() const
	{
		return time > 10.0;
	}

	inline bool is_response(sf::Uint8 type) const
	{
		return type == response;
	}
};

#include "client.hpp"

using std::string;

Message::Message(const string& mes, const sf::Font& font, unsigned int size, const sf::Color& color, float dur)
	: message {mes, font, size}, duration {dur}
{
	message.setColor(color);
}

MessageQ::MessageQ(const sf::Font& f)
	: font {f}
{
}

MessageQ::~MessageQ()
{
	clear();
}

void MessageQ::add(const string& message, Message::Severity severity)
{
	sf::Color color;
	unsigned int size;
	float duration;

	switch (severity)
	{
		case Message::Severity::LOW:
			color = sf::Color::White;
			size = 12;
			duration = 5;
			break;
		case Message::Severity::HIGH:
			color = sf::Color::Red;
			size = 14;
			duration = 5;
			break;
		case Message::Severity::CRITICAL:
			color = sf::Color::Red;
			size = 18;
			duration = -1;
			break;
		default:
			color = sf::Color::Black;
			size = 12;
			duration = 5;
	}

	messages.push_back(new Message(message, font, size, color, duration));
	messages.back()->set_pos(padding, bottom + padding);
	bottom += padding + messages.back()->get_height();
}

void MessageQ::step(float time)
{
	bool change {false};

	for (auto message = messages.begin(); message != messages.end();)
	{
		(*message)->step(time);
		if ((*message)->can_be_removed())
		{
			delete *message;
			message = messages.erase(message);
			change = true;
		}
		else
			message++;
	}

	// if a message was removed, need to reset all message positions
	if (change)
	{
		bottom = 0;
		for (auto message : messages)
		{
			message->set_pos(padding, bottom + padding);
			bottom += padding + message->get_height();
		}
	}
}

void MessageQ::clear()
{
	for (auto message : messages)
		delete message;
	messages.clear();

	bottom = 0;
}

void MessageQ::draw_on(sf::RenderWindow& window) const
{
	for (auto message : messages)
		message->draw_on(window);
}

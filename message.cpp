#include <SFML/Graphics.hpp>
#include "message.hpp"

using std::string;

Message::Message(const string& mes, const sf::Font& font, unsigned int size, const sf::Color& color) : message(mes, font, size)
{
	message.setColor(color);
}

MessageQ::MessageQ(const sf::Font& f) : font(f)
{
	bottom = 0;
}

void MessageQ::add(const string& message, severity_t severity)
{
	sf::Color color;
	unsigned int size;
	switch (severity)
	{
		case LOW:
			color = sf::Color::White;
			size = 12;
			break;
		case HIGH:
			color = sf::Color::Red;
			size = 18;
			break;
		default:
			color = sf::Color::Black;
			size = 12;
	}
	messages.push_back(new Message(message, font, size, color));
	messages.back()->set_pos(padding, bottom + padding);
	bottom += padding + messages.back()->get_height();
}

// TODO inefficient?
void MessageQ::age(float time)
{
	bool change = false;
	for (auto mess = messages.begin(); mess != messages.end();)
	{
		(*mess)->age(time);
		if ((*mess)->age() > 5)
		{
			mess = messages.erase(mess);
			change = true;
		}
		else
			mess++;
	}
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

void MessageQ::draw_on(sf::RenderWindow& window) const
{
	for (auto message : messages)
		message->draw_on(window);
}

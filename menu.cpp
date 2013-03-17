#include "bananagrams.hpp"

Menu::Menu(const std::string& ttl, const std::vector<std::string>& ents)
	: title(ttl, font, 50)
{
	title.setColor(sf::Color::White);
	auto bounds = title.getGlobalBounds();
	sf::Vector2f pos {-bounds.width / 2, 0};
	title.setPosition(pos);
	pos.y += bounds.height * 1.5;

	for (auto& entry : ents)
	{
		entries.push_back(sf::Text(entry, font, 30));
		entries.back().setColor(sf::Color(150, 150, 150));
		bounds = entries.back().getGlobalBounds();
		pos.x = -bounds.width / 2;
		pos.y += bounds.height * 1.5;
		entries.back().setPosition(pos);
	}

	highlighted = 0;
	highlight(0);
}

void Menu::highlight(unsigned int i)
{
	entries[highlighted].setColor(sf::Color(150, 150, 150));
	entries[highlighted = i].setColor(sf::Color::White);
}

void Menu::select(unsigned int i)
{
}

void Menu::draw_on(sf::RenderWindow& window) const
{
	window.draw(title);
	for (auto& entry: entries)
		window.draw(entry);
}

bool Menu::process_event(sf::Event& event)
{
	switch (event.type)
	{
		case sf::Event::KeyPressed:
			if (event.key.code == sf::Keyboard::Escape)
				finished = true;
			else if (highlighted > 0 && event.key.code == sf::Keyboard::Up)
				highlight(highlighted - 1);
			else if (highlighted < entries.size() - 1 && event.key.code == sf::Keyboard::Down)
				highlight(highlighted + 1);
			else if (event.key.code == sf::Keyboard::Return)
				select(highlighted);
			break;
		case sf::Event::MouseMoved:
		{
			sf::Vector2f mouse {(float)event.mouseMove.x, (float)event.mouseMove.y};
			for (unsigned int i = 0; i < entries.size(); i++)
				if (entries[i].getGlobalBounds().contains(mouse))
				{
					highlight(i);
					break;
				}
			break;
		}
		case sf::Event::MouseButtonPressed:
		{
			sf::Vector2f mouse {(float)event.mouseButton.x, (float)event.mouseButton.y};
			for (unsigned int i = 0; i < entries.size(); i++)
				if (entries[i].getGlobalBounds().contains(mouse))
				{
					highlight(i);
					break;
				}
			break;
		}
		case sf::Event::MouseButtonReleased:
			select(highlighted);
			break;
		default:
			break;
	}

	return false;
}

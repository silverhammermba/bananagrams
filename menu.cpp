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

	selected = 0;
	select(0);
}

void Menu::select(unsigned int i)
{
	entries[selected].setColor(sf::Color(150, 150, 150));
	entries[selected = i].setColor(sf::Color::White);
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
			else if (selected > 0 && event.key.code == sf::Keyboard::Up)
				select(selected - 1);
			else if (selected < entries.size() - 1 && event.key.code == sf::Keyboard::Down)
				select(selected + 1);
			break;
		case sf::Event::MouseMoved:
		{
			sf::Vector2f mouse {(float)event.mouseMove.x, (float)event.mouseMove.y};
			for (unsigned int i = 0; i < entries.size(); i++)
				if (entries[i].getGlobalBounds().contains(mouse))
				{
					select(i);
					break;
				}
			break;
		}
		default:
			break;
	}

	return false;
}

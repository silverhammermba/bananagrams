#include "bananagrams.hpp"

Menu::Menu(const sf::View& vw, const std::string& ttl, const std::vector<std::string>& ents, float size)
	: view(vw), title(ttl, font, size * 2)
{
	title.setColor(sf::Color::White);
	sf::Vector2f pos {-title.getGlobalBounds().width / 2, 0};
	title.setPosition(pos);
	pos.y += size * 3;
	float left = pos.x;

	for (auto& entry : ents)
	{
		entries.push_back(sf::Text(entry, font, size));
		entries.back().setColor(sf::Color(150, 150, 150));
		pos.x = -entries.back().getGlobalBounds().width / 2;
		entries.back().setPosition(pos);

		if (pos.x < left)
			left = pos.x;

		pos.y += size * 1.5;
	}

	float shift = (view.getSize().y - entries.back().getGlobalBounds().top - title.getGlobalBounds().top + entries.back().getGlobalBounds().height) / 2;

	title.move(view.getCenter().x, shift);

	for (auto& entry : entries)
		entry.move(view.getCenter().x, shift);

	background.setFillColor(sf::Color(0, 0, 0, 200));
	background.setSize({-2 * left + size, view.getSize().y + size});
	background.setPosition(view.getCenter().x + left - size / 2, size / -2);

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
	window.draw(background);
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
			else if (event.key.code == sf::Keyboard::Up)
				highlight(highlighted == 0 ? entries.size() - 1 : highlighted - 1);
			else if (event.key.code == sf::Keyboard::Down)
				highlight(highlighted == entries.size() - 1 ? 0 : highlighted + 1);
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

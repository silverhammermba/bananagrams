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
		entries.back().setColor(sf::Color::White);
		bounds = entries.back().getGlobalBounds();
		pos.x = -bounds.width / 2;
		pos.y += bounds.height * 1.5;
		entries.back().setPosition(pos);
	}
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
			break;
		default:
			break;
	}

	return false;
}

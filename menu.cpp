#include "bananagrams.hpp"

Submenu::Submenu(const sf::View& vw, Submenu* p, const std::string& ttl, float sz)
	: view(vw), parent(p), title(ttl, font, sz * 2)
{
	size = sz;
	title.setColor(sf::Color::White);

	background.setFillColor(sf::Color(0, 0, 0, 200));

	// TODO dangerous if no entries are added
	highlighted = 0;
}

void Submenu::add_entry(const std::string& entry, Submenu* sub)
{
	sf::Vector2f pos {-title.getGlobalBounds().width / 2, 0};
	title.setPosition(pos);
	pos.y += size * 3;
	float left = pos.x;

	entries.push_back(sf::Text(entry, font, size));
	entries.back().setColor(sf::Color(150, 150, 150));

	for (auto& entry : entries)
	{
		pos.x = -entry.getGlobalBounds().width / 2;
		entry.setPosition(pos);

		if (pos.x < left)
			left = pos.x;

		pos.y += size * 1.5;
	}

	float shift = (view.getSize().y - entries.back().getGlobalBounds().top - title.getGlobalBounds().top + entries.back().getGlobalBounds().height) / 2;

	title.move(view.getCenter().x, shift);

	for (auto& entry : entries)
		entry.move(view.getCenter().x, shift);

	background.setSize({-2 * left + size, view.getSize().y + size});
	background.setPosition(view.getCenter().x + left - size / 2, size / -2);

	submenus.push_back(sub);

	if (entries.size() == 1)
		highlight(0);
}

void Submenu::highlight(unsigned int i)
{
	entries[highlighted].setColor(sf::Color(150, 150, 150));
	entries[highlighted = i].setColor(sf::Color::White);
}

void Submenu::highlight_coords(float x, float y)
{
	sf::Vector2f mouse {x, y};
	for (unsigned int i = 0; i < entries.size(); i++)
		if (entries[i].getGlobalBounds().contains(mouse))
		{
			highlight(i);
			break;
		}
}

void Submenu::draw_on(sf::RenderWindow& window) const
{
	window.draw(background);
	window.draw(title);
	for (auto& entry: entries)
		window.draw(entry);
}

Menu::Menu(Submenu& root)
{
	submenu = &root;
}

void Menu::draw_on(sf::RenderWindow& window) const
{
	submenu->draw_on(window);
}

bool Menu::process_event(sf::Event& event)
{
	switch (event.type)
	{
		case sf::Event::KeyPressed:
			switch (event.key.code)
			{
				case sf::Keyboard::Escape:
					if (submenu->get_parent() == nullptr)
						finished = true;
					else
						submenu = submenu->get_parent();
					break;
				case sf::Keyboard::Up:
					submenu->highlight_prev();
					break;
				case sf::Keyboard::Down:
					submenu->highlight_next();
					break;
				case sf::Keyboard::Return:
				{
					Submenu* sub = submenu->select();
					if (sub != nullptr)
						submenu = sub;
					break;
				}
				default:
					break;
			}
			break;
		case sf::Event::MouseMoved:
			submenu->highlight_coords((float)event.mouseMove.x, (float)event.mouseMove.y);
			break;
		case sf::Event::MouseButtonPressed:
			submenu->highlight_coords((float)event.mouseButton.x, (float)event.mouseButton.y);
			break;
		case sf::Event::MouseButtonReleased:
		{
			Submenu* sub = submenu->select();
			if (sub != nullptr)
				submenu = sub;
			break;
		}
		default:
			break;
	}

	return false;
}

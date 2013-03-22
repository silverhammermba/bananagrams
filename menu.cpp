#include "bananagrams.hpp"

Entry::Entry(const std::string& txt)
	: text(txt, font, PPB * 1.5)
{
	lowlight();
}

float Entry::get_width() const
{
	return text.getGlobalBounds().width;
}

float Entry::get_height() const
{
	return PPB * 1.5;
}

sf::FloatRect Entry::bounds() const
{
	return text.getGlobalBounds();
}

void Entry::set_menu_pos(float center, float width, float top)
{
	text.setPosition(center + text.getGlobalBounds().width / -2, top);
}

void Entry::highlight()
{
	text.setColor(sf::Color::White);
}

void Entry::lowlight()
{
	text.setColor(sf::Color(150, 150, 150));
}

void Entry::draw_on(sf::RenderWindow& window) const
{
	window.draw(text);
}

MenuEntry::MenuEntry(std::string txt, MenuSystem& sys, Menu* sub)
	: Entry(txt), system(sys)
{
	submenu = sub;
}

void MenuEntry::select()
{
	if (submenu == nullptr)
		system.close(); // TODO why do we need to double tap Esc when closing this way?
	else
		system.set_menu(*submenu);
}

QuitEntry::QuitEntry(std::string txt, sf::RenderWindow& win)
	: Entry(txt), window(win)
{}

void QuitEntry::select()
{
	window.close();
}

// TODO react to changing view
Menu::Menu(const sf::View& vw, MenuSystem& sys, Menu* p, const std::string& ttl)
	: view(vw), system(sys), parent(p), title(ttl, font, PPB * 2.0)
{
	title.setColor(sf::Color::White);

	background.setFillColor(sf::Color(0, 0, 0, 200));

	// TODO dangerous if no entries are added
	highlighted = entries.begin();
}

void Menu::add_entry(std::list<Entry*>::iterator it, Entry* entry)
{
	float max_width = title.getGlobalBounds().width;

	entries.insert(it, entry);

	for (auto entry : entries)
	{
		float width = entry->get_width();
		if (width > max_width)
			max_width = width;
	}

	float height = PPB * 2.5 + (entries.size() - 1) * PPB * 1.5 + entries.back()->get_height();
	float shift = (view.getSize().y - height) / 2;

	title.setPosition(view.getCenter().x + title.getGlobalBounds().width / -2, shift);

	unsigned int i = 0;
	for (auto entry : entries)
		entry->set_menu_pos(view.getCenter().x, max_width, shift + PPB * 2.5 + i++ * PPB * 1.5);

	background.setSize({max_width + PPB, view.getSize().y + PPB});
	background.setPosition(view.getCenter().x + background.getSize().x / -2, PPB / -2.0);

	if (entries.size() == 1)
		highlight(entries.begin());
}

void Menu::highlight(std::list<Entry*>::iterator it)
{
	if (highlighted != entries.end())
		(*highlighted)->lowlight();
	(*(highlighted = it))->highlight();
}

void Menu::highlight_prev()
{
	auto it = highlighted;
	if (it == entries.begin())
		it = entries.end();
	--it; // TODO dangerous if menu is empty!
	highlight(it);
}

void Menu::highlight_next()
{
	auto it = highlighted;
	++it; // TODO dangerous if menu is empty!
	if (it == entries.end())
		it = entries.begin();
	highlight(it);
}

void Menu::highlight_coords(float x, float y)
{
	sf::Vector2f mouse {x, y};
	for (auto it = entries.begin(); it != entries.end(); it++)
		if ((*it)->bounds().contains(mouse))
		{
			highlight(it);
			break;
		}
}

void Menu::draw_on(sf::RenderWindow& window) const
{
	window.draw(background);
	window.draw(title);
	for (auto entry: entries)
		entry->draw_on(window);
}

bool Menu::process_event(sf::Event& event)
{
	switch (event.type)
	{
		case sf::Event::KeyPressed:
			switch (event.key.code)
			{
				case sf::Keyboard::Escape:
					if (parent != nullptr)
						system.set_menu(*parent);
					else
						system.close();
					break;
				case sf::Keyboard::Up:
					highlight_prev();
					break;
				case sf::Keyboard::Down:
					highlight_next();
					break;
				case sf::Keyboard::Return:
					(*highlighted)->select();
					break;
				default:
					break;
			}
			break;
		case sf::Event::MouseMoved:
		{
			highlight_coords(event.mouseMove.x, event.mouseMove.y);
			break;
		}
		case sf::Event::MouseButtonPressed:
		{
			highlight_coords(event.mouseButton.x, event.mouseButton.y);
			break;
		}
		case sf::Event::MouseButtonReleased:
			(*highlighted)->select();
			break;
		default:
			(*highlighted)->process_event(event);
			break;
	}

	return false;
}

bool MenuSystem::process_event(sf::Event& event)
{
	menu_p->process_event(event);

	return false;
}

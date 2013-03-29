#include "bananagrams.hpp"

// TODO put these in Entry
static const sf::Color ACTIVE(255, 255, 255);
static const sf::Color INACTIVE(150, 150, 150);

Entry::Entry(const std::string& txt)
	: text(txt, font, PPB)
{
	lowlight();
}

float Entry::get_width() const
{
	return text.getGlobalBounds().width;
}

float Entry::get_height() const
{
	return PPB;
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
	text.setColor(ACTIVE);
}

void Entry::lowlight()
{
	text.setColor(INACTIVE);
}

void Entry::draw_on(sf::RenderWindow& window) const
{
	window.draw(text);
}

MenuEntry::MenuEntry(const std::string& txt, MenuSystem& sys, Menu* sub)
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

SolitaireEntry::SolitaireEntry(const std::string& txt, MenuSystem& sys, TextEntry& dict, MultiEntry& mult, Game& g)
	: Entry(txt), system(sys), dict_entry(dict), multiplier(mult), game(g)
{
}

void SolitaireEntry::select()
{
	system.close();
	// TODO display loading text
	int mul = 1;
	int div = 1;
	if (multiplier.get_choice() == 0)
		div = 2;
	else
		mul = multiplier.get_choice();
	if (!game.restart(dict_entry.get_string(), mul, div))
		system.open();
}

TextEntry::TextEntry(const std::string& txt, float mbw, const std::string& def, const std::string& def_display)
	: Entry(txt), box(), input(def_display, font, (PPB * 2) / 3.0), str(def), default_display(def_display), default_str(def)
{
	min_box_width = mbw;

	// get heights and shifts
	text.setString("A");
	b_height = text.getGlobalBounds().height;
	text.setString(txt);
	input.setString("A");
	auto i_bounds = input.getGlobalBounds();
	i_height = i_bounds.height;
	shift = input.getPosition().y - i_bounds.top;
	input.setString(def_display);

	// set up input box
	box.setFillColor(sf::Color::Transparent);
	box.setOutlineColor(INACTIVE);
	box.setOutlineThickness(2);

	input.setColor(INACTIVE);
	selected = false;
}

float TextEntry::get_width() const
{
	// TODO set minimum width of box?
	return text.getGlobalBounds().width + PPB * 0.5 + min_box_width;
}

sf::FloatRect TextEntry::bounds() const
{
	auto tb = text.getGlobalBounds();
	auto bb = box.getGlobalBounds();
	sf::FloatRect b(tb.left, bb.top, bb.left + bb.width - tb.left, bb.height);
	return b;
}

// center input in box
void TextEntry::set_input_pos()
{
	auto i_pos = input.getPosition();
	auto i_bounds = input.getGlobalBounds();
	auto b_pos = box.getPosition();
	auto b_size = box.getSize();
	input.setPosition(b_pos.x + (b_size.x - i_bounds.width) / 2 - (i_bounds.left - i_pos.x), b_pos.y + (b_height - i_height) / 2 + shift);
}

void TextEntry::set_menu_pos(float center, float width, float top)
{
	text.setPosition(center - width / 2, top);
	auto tb = text.getGlobalBounds();
	box.setSize({(float)(width - tb.width - PPB * 0.5), b_height});
	box.setPosition(center - width / 2 + tb.width + PPB * 0.5, tb.top + tb.height - b_height);
	set_input_pos();
}

void TextEntry::highlight()
{
	box.setOutlineColor(ACTIVE);
	Entry::highlight();
}

void TextEntry::lowlight()
{
	selected = false;
	box.setOutlineColor(INACTIVE);
	input.setColor(INACTIVE);
	Entry::lowlight();
}

void TextEntry::select()
{
	selected = !selected;
	if (selected)
	{
		input.setColor(ACTIVE);
		if (str == default_str)
		{
			str = "";
			input.setString(str);
		}
	}
	else
	{
		input.setColor(INACTIVE);
		// TODO some string processing?
		if (str == "")
		{
			str = default_str;
			input.setString(default_display);
			set_input_pos();
		}
	}
}

bool TextEntry::process_event(sf::Event& event)
{
	if (selected)
	{
		if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Key::BackSpace)
		{
			str = str.substr(0, str.size() - 1); // TODO better way to remove last?
			input.setString(str);
			set_input_pos();
		}
		else if (event.type == sf::Event::TextEntered)
		{
			// TODO I am a terrible person
			char ch = (char)event.text.unicode;
			if (ch >= ' ' && ch <= '~')
			{
				std::stringstream fuck;
				fuck << ch;
				str += fuck.str();
				input.setString(str);
				set_input_pos();
			}
		}
	}

	return true;
}

void TextEntry::draw_on(sf::RenderWindow& window) const
{
	window.draw(text);
	window.draw(box);
	window.draw(input);
}

MultiEntry::MultiEntry(const std::string& txt, const std::vector<std::string>& ch, unsigned int def)
	: Entry(txt), choices(ch), chooser("", font, PPB)
{
	// TODO make sure default is good
	choice = def;
	max_width = 0;

	// find max width
	for (auto choice : choices)
	{
		chooser.setString("< " + choice + " >");
		float w = chooser.getGlobalBounds().width;
		if (w > max_width)
			max_width = w;
	}

	// TODO make these arrows nice. maybe use unicode?
	chooser.setString("< " + choices[0] + " >");
	lowlight();
}

float MultiEntry::get_width() const
{
	return text.getGlobalBounds().width + PPB + max_width;
}

sf::FloatRect MultiEntry::bounds() const
{
	auto tb = text.getGlobalBounds();
	auto cb = chooser.getGlobalBounds();
	sf::FloatRect b(tb.left, std::min(tb.top, cb.top), cb.left + cb.width - tb.left, std::max(tb.height, cb.height));
	return b;
}

void MultiEntry::update_choice()
{
	chooser.setString("< " + choices[choice] + " >");
	chooser.setPosition(text.getGlobalBounds().left + text.getGlobalBounds().width + PPB + (set_width - chooser.getGlobalBounds().width) / 2, chooser.getPosition().y);
}

void MultiEntry::set_menu_pos(float center, float width, float top)
{
	text.setPosition(center - width / 2, top);
	chooser.setPosition(chooser.getPosition().x, top);
	set_width = width - PPB - text.getGlobalBounds().width;
	update_choice();
}

void MultiEntry::highlight()
{
	chooser.setColor(ACTIVE);
	Entry::highlight();
}

void MultiEntry::lowlight()
{
	chooser.setColor(INACTIVE);
	Entry::lowlight();
}

bool MultiEntry::process_event(sf::Event& event)
{
	if (event.type == sf::Event::KeyPressed)
	{
		if (choice > 0 && event.key.code == sf::Keyboard::Key::Left)
			--choice;
		else if (choice < choices.size() - 1 && event.key.code == sf::Keyboard::Key::Right)
			++choice;
		update_choice();
	}

	return true;
}

void MultiEntry::draw_on(sf::RenderWindow& window) const
{
	window.draw(chooser);
	Entry::draw_on(window);
}

std::string cmd2menu(const std::string& cmd)
{
	std::string menu(cmd);

	for (unsigned int i = 0; i < menu.length(); i++)
	{
		menu[i] = std::toupper(menu[i]);
		if (menu[i] == '_')
			menu[i] = ' ';
	}

	return menu;
}

ControlEntry::ControlEntry(const std::string& cmd, const sf::Event::KeyEvent& k)
	: Entry(cmd2menu(cmd)), command(cmd), key(k), box(), key_text(key2str(k), font, (PPB * 2) / 3.0)
{
	// get heights and shifts
	text.setString("A");
	b_shift = text.getGlobalBounds().top;
	b_height = text.getGlobalBounds().height;
	text.setString(cmd2menu(cmd));
	key_text.setString("A");
	auto i_bounds = key_text.getGlobalBounds();
	i_height = i_bounds.height;
	shift = key_text.getPosition().y - i_bounds.top;
	key_text.setString(key2str(key));

	// set up key_text box
	box.setFillColor(sf::Color::Transparent);
	box.setOutlineColor(INACTIVE);
	box.setOutlineThickness(2);

	key_text.setColor(INACTIVE);
	selected = false;
}

float ControlEntry::get_width() const
{
	// TODO how to make these align and shit?
	return text.getGlobalBounds().width + PPB * 0.5 + PPB * 5;
}

sf::FloatRect ControlEntry::bounds() const
{
	auto tb = text.getGlobalBounds();
	auto bb = box.getGlobalBounds();
	sf::FloatRect b(tb.left, bb.top, bb.left + bb.width - tb.left, bb.height);
	return b;
}

// center key_text in box
void ControlEntry::set_input_pos()
{
	auto i_pos = key_text.getPosition();
	auto i_bounds = key_text.getGlobalBounds();
	auto b_pos = box.getPosition();
	auto b_size = box.getSize();
	key_text.setPosition(b_pos.x + (b_size.x - i_bounds.width) / 2 - (i_bounds.left - i_pos.x), b_pos.y + (b_height - i_height) / 2 + shift);
}

void ControlEntry::set_menu_pos(float center, float width, float top)
{
	text.setPosition(center - width / 2, top);
	auto tb = text.getGlobalBounds();
	box.setSize({(float)(PPB * 5), b_height});
	box.setPosition(center + width / 2 - PPB * 5, top + b_shift);
	set_input_pos();
}

void ControlEntry::highlight()
{
	box.setOutlineColor(ACTIVE);
	Entry::highlight();
}

void ControlEntry::lowlight()
{
	selected = false;
	box.setOutlineColor(INACTIVE);
	key_text.setColor(INACTIVE);
	key_text.setString(key2str(key));
	set_input_pos();
	Entry::lowlight();
}

void ControlEntry::select()
{
	selected = !selected;
	if (selected)
	{
		key_text.setColor(ACTIVE);
		key_text.setString("press a key...");
	}
	else
	{
		key_text.setColor(INACTIVE);
		key_text.setString(key2str(key));
	}
	set_input_pos();
}

bool ControlEntry::process_event(sf::Event& event)
{
	if (selected)
	{
		if (event.type == sf::Event::KeyReleased)
		{
			if (event.key.code == sf::Keyboard::Key::LAlt || event.key.code == sf::Keyboard::Key::RShift)
				event.key.alt = false;
			if (event.key.code == sf::Keyboard::Key::LControl || event.key.code == sf::Keyboard::Key::RShift)
				event.key.control = false;
			if (event.key.code == sf::Keyboard::Key::LShift || event.key.code == sf::Keyboard::Key::RShift)
				event.key.shift = false;
			// TODO don't allow certain events?
			if (event.key.code != sf::Keyboard::Key::Escape)
				key = event.key;
			lowlight();
			highlight();
		}

		return false;
	}

	return true;
}

void ControlEntry::draw_on(sf::RenderWindow& window) const
{
	window.draw(text);
	window.draw(box);
	window.draw(key_text);
}


QuitEntry::QuitEntry(const std::string& txt, sf::RenderWindow& win)
	: Entry(txt), window(win)
{}

void QuitEntry::select()
{
	window.close();
}

// TODO react to changing view
Menu::Menu(MenuSystem& sys, Menu* p, const std::string& ttl)
	: system(sys), parent(p), title(ttl, font, PPB * 1.5)
{
	title.setColor(ACTIVE);

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

	float height = PPB * 2.0 + (entries.size() - 1) * PPB + entries.back()->get_height();
	float shift = (gui_view.getSize().y - height) / 2;

	title.setPosition(gui_view.getCenter().x + title.getGlobalBounds().width / -2, shift);

	unsigned int i = 0;
	for (auto entry : entries)
		entry->set_menu_pos(gui_view.getCenter().x, max_width, shift + PPB * 2.0 + i++ * PPB);

	background.setSize({max_width + PPB, gui_view.getSize().y + PPB});
	background.setPosition(gui_view.getCenter().x + background.getSize().x / -2, PPB / -2.0);

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
	// only handle event if the highlighted entry tells you to
	if (!(*highlighted)->process_event(event))
		return false;

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
				default:
					break;
			}
			break;
		case sf::Event::KeyReleased:
			switch (event.key.code)
			{
				case sf::Keyboard::Return:
					(*highlighted)->select();
					break;
				default:
					break;
			}
			break;
		case sf::Event::MouseMoved:
			highlight_coords(event.mouseMove.x, event.mouseMove.y);
			break;
		case sf::Event::MouseButtonPressed:
			highlight_coords(event.mouseButton.x, event.mouseButton.y);
			break;
		case sf::Event::MouseButtonReleased:
			(*highlighted)->select();
			break;
		default:
			break;
	}

	return false;
}

bool MenuSystem::process_event(sf::Event& event)
{
	menu_p->process_event(event);

	return false;
}

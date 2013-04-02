#include "bananagrams.hpp"

// embedded resources
#include "icon.hpp"

using std::cerr;
using std::endl;
using std::string;
using std::stringstream;
using std::vector;

// objects needed globally
sf::Font font;
sf::RenderTexture tile_texture[26];
sf::View gui_view;

// class for handling game-related events
class WindowEvents : public InputReader
{
	State* state;
public:
	WindowEvents(State* s)
		: state {s}
	{
	}

	virtual bool process_event(sf::Event& event)
	{
		if (event.type == sf::Event::Closed)
		{
			state->window->close();

			finished = true;
			return false;
		}
		else if (event.type == sf::Event::Resized)
		{
			gui_view.setSize(event.size.width, event.size.height);
			gui_view.setCenter(event.size.width / 2.0, event.size.height / 2.0);
			state->grid_view->setSize(event.size.width, event.size.height);
			state->grid_view->zoom(state->zoom);
		}
		return true;
	}
};

int main()
{
	// load resources
	if (!font.loadFromFile("/usr/share/fonts/TTF/DejaVuSans.ttf"))
	{
		cerr << "Couldn't find font /usr/share/fonts/TTF/DejaVuSans.ttf!\n";
		return 1;
	}

	KeyControls controls;
	// TODO store last dictionary name, last resolution settings, etc.
	controls.load_from_file("config.yaml");

	std::srand((unsigned int)std::time(nullptr));

	sf::RenderWindow window {sf::VideoMode(1280, 720), "Bananagrams"};
	window.setIcon(32, 32, icon);
	window.setVerticalSyncEnabled(true);

	gui_view = window.getDefaultView();
	sf::View grid_view {window.getDefaultView()};
	grid_view.setCenter(PPB / 2.0, PPB / 2.0);

	vector<InputReader*> input_readers;

	State state;
	state.window = &window;
	state.grid_view = &grid_view;
	state.zoom = 1;

	WindowEvents win_events {&state};
	input_readers.push_back(&win_events);

	sf::Color background {22, 22, 22};

	sf::Clock clock;

	sf::Text loading_text {"for Monica", font, 30};
	loading_text.setColor(sf::Color(255, 255, 255, 0));
	sf::FloatRect bounds {loading_text.getGlobalBounds()};
	sf::Vector2f center = gui_view.getCenter();
	loading_text.setPosition(center.x + bounds.width / -2, center.y + bounds.height / -2);

	// dedication
	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			for (auto r = input_readers.begin(); r != input_readers.end();)
			{
				bool cont {(*r)->process_event(event)};
				if ((*r)->is_finished())
					r = input_readers.erase(r);
				else
					r++;
				if (!cont)
					break;
			}
		}

		float elapsed {clock.getElapsedTime().asSeconds()};

		if (elapsed < 1)
			loading_text.setColor(sf::Color(255, 255, 255, 255 * elapsed));
		else if (elapsed > 2)
			loading_text.setColor(sf::Color(255, 255, 255, 255 - 255 * (elapsed - 2)));
		else
			loading_text.setColor(sf::Color::White);

		if (elapsed > 3)
			break;

		window.clear(background);
		window.draw(loading_text);
		window.display();
	}
	if (!window.isOpen())
		return 0;

	loading_text.setColor(sf::Color::White);

	loading_text.setString("Generating textures...");
	sf::FloatRect ltbounds {loading_text.getGlobalBounds()};
	loading_text.setPosition(center.x - ltbounds.width / 2, center.y - ltbounds.height * 2.5);

	// generate textures
	{
		// for generating tiles
		float padding {PPB / 2.f};
		char load_char {'A'};
		float miny {0};
		float height {0};
		bool done_y {false};
		vector<sf::Sprite> loaded;

		while (window.isOpen())
		{
			sf::Event event;
			while (window.pollEvent(event))
			{
				for (auto r = input_readers.begin(); r != input_readers.end();)
				{
					bool cont {(*r)->process_event(event)};
					if ((*r)->is_finished())
						r = input_readers.erase(r);
					else
						r++;
					if (!cont)
						break;
				}
			}

			// TODO some kind of depth would help for stack appearance
			// TODO antialiase these
			if (!tile_texture[load_char - 'A'].create(PPB, PPB))
			{
				cerr << "Failed to allocate tile texture!\n";
				return 1;
			}

			// create text
			stringstream str;
			str << load_char;
			sf::Text letter {str.str(), font, (unsigned int)((PPB * 2) / 3.0)};
			letter.setColor(sf::Color::Black);

			// center
			sf::FloatRect bounds = letter.getGlobalBounds();
			float minx {bounds.left};
			float width {bounds.width};
			// only calculate center height for A, to fix letters like Q
			if (!done_y)
			{
				miny = bounds.top;
				height = bounds.height;
			}
			done_y = true;
			letter.setPosition((PPB - width) / 2.0 - minx, (PPB - height) / 2.0 - miny);

			float diam {PPB / 4.f};
			float rad {PPB / 8.f};
			// draw tile
			tile_texture[load_char - 'A'].clear(sf::Color(0, 0, 0, 0));

			sf::RectangleShape rect;
			rect.setFillColor(sf::Color(255, 255, 175));

			rect.setSize(sf::Vector2f {PPB - diam, PPB});
			rect.setPosition(rad, 0);
			tile_texture[load_char - 'A'].draw(rect);
			rect.setSize(sf::Vector2f {PPB, PPB - diam});
			rect.setPosition(0, rad);
			tile_texture[load_char - 'A'].draw(rect);

			sf::CircleShape circle {rad};
			circle.setFillColor(sf::Color(255, 255, 175));

			circle.setPosition(0, 0);
			tile_texture[load_char - 'A'].draw(circle);
			circle.setPosition(PPB - diam, 0);
			tile_texture[load_char - 'A'].draw(circle);
			circle.setPosition(0, PPB - diam);
			tile_texture[load_char - 'A'].draw(circle);
			circle.setPosition(PPB - diam, PPB - diam);
			tile_texture[load_char - 'A'].draw(circle);

			tile_texture[load_char - 'A'].draw(letter);
			tile_texture[load_char - 'A'].display();

			// display generated texture
			loaded.push_back(sf::Sprite(tile_texture[load_char - 'A'].getTexture()));

			window.clear(background);

			window.setView(gui_view);
			float vwidth {gui_view.getSize().x};
			sf::Vector2f vcenter = gui_view.getCenter();

			loading_text.setPosition(vcenter.x - ltbounds.width / 2.5, vcenter.y - ltbounds.height * 2.5);
			window.draw(loading_text);

			unsigned int i {0};
			for (auto& sprite : loaded)
			{
				sprite.setPosition((i * (vwidth - 2 * padding - PPB)) / 25.0 + vcenter.x - vwidth / 2 + padding, vcenter.y + PPB / -2.0);
				window.draw(sprite);
				++i;
			}

			window.display();

			// we're done
			if(++load_char > 'Z')
				break;
		}
	}
	if (!window.isOpen())
		return 0;

	MenuSystem current;
	Menu main {current, nullptr, "BANANAGRAMS"};
	MenuEntry solitaire {"SOLITAIRE", current};
	MenuEntry customize {"CONTROLS", current};
	MenuEntry quit {"QUIT", current};
	main.append_entry(&solitaire);
	main.append_entry(&customize);
	main.append_entry(&quit);

	Menu solitaire_opts {current, &main, "SOLITAIRE"};
	solitaire.submenu = &solitaire_opts;

	Game game;
	TextEntry dict_entry {"DICTIONARY", PPB * 8, "dictionary.txt"};
	MultiEntry multiplier {"BUNCH x", {"1/2", "1", "2", "3", "4"}, 1};
	SolitaireEntry start {"START GAME", current, dict_entry, multiplier, game};

	solitaire_opts.append_entry(&start);
	solitaire_opts.append_entry(&dict_entry);
	solitaire_opts.append_entry(&multiplier);

	Menu control_opts {current, &main, "CONTROLS"};
	customize.submenu = &control_opts;

	// TODO need scrolling menus for this to work...
	for (auto& pair : controls.get_binds())
		if (controls.is_rebindable(pair.second))
			control_opts.append_entry(new ControlEntry(pair.second, pair.first));

	Menu confirm_quit {current, &main, "Really quit?"};
	quit.submenu = &confirm_quit;
	QuitEntry yes {"YES", window};
	MenuEntry no {"NO", current, &main};
	confirm_quit.append_entry(&yes);
	confirm_quit.append_entry(&no);

	current.set_menu(main);
	input_readers.push_back(&current);

	loading_text.setString("Loading dictionary...");
	ltbounds = loading_text.getGlobalBounds();
	loading_text.setPosition(center.x - ltbounds.width / 2, center.y - ltbounds.height * 2.5);
	window.clear(background);
	window.draw(loading_text);
	window.display();

	// stuff for game loop
	Cursor cursor {{1, 1}, PPB / 16.f, sf::Color::Transparent, sf::Color {0, 200, 0}};
	Cursor mcursor {{1, 1}, PPB / 16.0, sf::Color::Transparent, sf::Color {0, 200, 0, 80}};

	sf::Vector2i last {-1, 0};
	sf::Vector2i next {0, 0};

	// mouse press and release positions
	sf::Vector2i sel1;
	sf::Vector2i sel2;
	Cursor selection {{1, 1}, 1, sf::Color {255, 255, 255, 25}, sf::Color::White};

	// mouse
	state.pos[0] = 0;
	state.pos[1] = 0;
	state.update = false;
	state.mremove = false;
	state.wheel_delta = 0;
	state.start_selection = false;
	state.end_selection = false;

	MouseControls mouse {&state};
	input_readers.push_back(&mouse);

	input_readers.push_back(&controls);

	Typer typer;
	input_readers.push_back(&typer);

	// game loop
	while (window.isOpen())
	{
		// needed for mouse cursor position and zoom control
		auto wsize = window.getSize();
		// needed for cursor positions
		auto gsize = grid_view.getSize();
		auto center = grid_view.getCenter();

		sf::Event event;
		while (window.pollEvent(event))
		{
			for (auto r = input_readers.begin(); r != input_readers.end();)
			{
				bool cont {(*r)->process_event(event)};
				if ((*r)->is_finished())
					r = input_readers.erase(r);
				else
					++r;
				if (!cont)
					break;
			}
		}

		// TODO only redraw window if you need to? sleep when no events received maybe...

		// mouse moved
		if (state.update)
		{
			// TODO refactor
			// update mouse cursor position
			mcursor.set_pos({(int)std::floor(((state.pos[0] * gsize.x) / wsize.x + center.x - (gsize.x / 2)) / PPB), (int)std::floor(((state.pos[1] * gsize.y) / wsize.y + center.y - (gsize.y / 2)) / PPB)});

			if (game.buffer != nullptr)
				game.buffer->set_pos(mcursor.get_pos());

			state.update = false;
		}

		// left click
		if (state.start_selection)
		{
			game.selecting = true;
			game.selected = false;
			sel1 = mcursor.get_pos();
			state.start_selection = false;
		}

		// if left click held down
		if (game.selecting)
		{
			sel2 = mcursor.get_pos();
			// update selection rect
			selection.set_size({(unsigned int)std::abs(sel1.x - sel2.x) + 1, (unsigned int)std::abs(sel1.y - sel2.y) + 1});
			selection.set_pos({std::min(sel1.x, sel2.x), std::min(sel1.y, sel2.y)});
		}

		// if left click release
		if (state.end_selection)
		{
			game.selecting = false;
			game.selected = true;

			// if the selection was only 1 square, do something else
			if (selection.get_size() == sf::Vector2u(1, 1))
			{
				game.selected = false;
				// move cursor to mouse cursor
				cursor.set_pos(mcursor.get_pos());

				if (controls["quick_place"])
				{
					// look for remaining tiles
					char last {'A' - 1};
					for (char ch = 'A'; ch <= 'Z'; ch++)
					{
						if (game.hand.has_any(ch))
						{
							if (last < 'A')
								last = ch;
							else
							{
								last = 'Z' + 1;
								break;
							}
						}
					}

					if (last < 'A')
						game.messages.add("You do not have any tiles.", Message::Severity::LOW);
					else if (last > 'Z')
						game.messages.add("You have too many letters to place using the mouse.", Message::Severity::HIGH);
					else
					{
						Tile* tile {game.grid.swap(cursor.get_pos(), game.hand.remove_tile(last))};

						if (tile != nullptr)
							game.hand.add_tile(tile);
					}
				}
			}
			state.end_selection = false;
		}

		if (controls["cut"])
		{
			if (game.buffer == nullptr)
			{
				if (game.selected)
				{
					game.buffer = new CutBuffer(game.grid, std::min(sel1.x, sel2.x), std::min(sel1.y, sel2.y), selection.get_size());

					if (game.buffer->is_empty())
					{
						game.messages.add("Nothing selected.", Message::Severity::LOW);
						game.clear_buffer();
					}
				}
				else
					game.messages.add("Nothing selected.", Message::Severity::LOW);

				game.selected = false;
			}
			else
			{
				game.buffer->clear(game.hand);
				game.clear_buffer();
				game.messages.add("Added cut tiles back to your hand.", Message::Severity::LOW);
			}
		}

		if (controls["flip"])
		{
			if (game.buffer != nullptr)
				game.buffer->transpose();
		}

		if (controls["paste"])
		{
			if (game.buffer != nullptr)
			{
				game.buffer->paste(game.grid, game.hand);
				game.clear_buffer();
			}
			else
				game.messages.add("Cannot paste: no tiles were cut.", Message::Severity::LOW);
		}

		if (state.mremove)
		{
			// remove tile
			Tile* tile {game.grid.remove(mcursor.get_pos())};
			if (tile != nullptr)
				game.hand.add_tile(tile);
		}

		if (controls["dump"])
		{
			if (game.bunch.size() >= 3)
			{
				Tile* dumped {game.grid.remove(cursor.get_pos())};
				if (dumped == nullptr)
					game.messages.add("You need to select a tile to dump.", Message::Severity::LOW);
				else
				{
					// take three
					for (unsigned int i = 0; i < 3; i++)
					{
						Tile* tile {game.bunch.back()};
						game.bunch.pop_back();
						game.hand.add_tile(tile);
					}

					random_insert<Tile*>(game.bunch, dumped);
				}
			}
			else
				game.messages.add("There are not enough tiles left to dump!", Message::Severity::HIGH);

		}

		if (controls["peel"])
		{
			bool spent {true};
			for (char ch = 'A'; ch <= 'Z'; ch++)
				if (game.hand.has_any(ch))
				{
					spent = false;
					break;
				}
			if (spent)
			{
				vector<string> mess;
				if (game.grid.is_valid(game.dictionary, mess))
				{
					if (game.bunch.size() > 0)
					{
						Tile* tile {game.bunch.back()};
						game.bunch.pop_back();
						game.hand.add_tile(tile);
						for (auto message : mess)
							game.messages.add(message, Message::Severity::LOW);
					}
					else
						game.messages.add("You win!", Message::Severity::LOW);
				}
				else
					for (auto message : mess)
						game.messages.add(message, Message::Severity::HIGH);
			}
			else
				game.messages.add("You have not used all of your letters.", Message::Severity::HIGH);
		}

		// if backspace
		if (controls["remove"])
		{
			const sf::Vector2i& pos = cursor.get_pos();
			// TODO DRY off autoadvancing
			// if the cursor is ahead of the last added character, autoadvance
			if (pos == last + next)
			{
				cursor.set_pos(last);
				last -= next;
			}
			// TODO guess based on cursor movement first?
			// else if you are not near the last character and the space is empty, try to autoadvance
			else if (game.grid.get(pos) == nullptr)
			{
				// TODO last case covers these first ones
				// if right of a tile
				if (game.grid.get(pos - X) != nullptr)
				{
					next = X;
					cursor.move(-next);
					last = cursor.get_pos() - next;
				}
				// if below a tile
				else if (game.grid.get(pos - Y) != nullptr)
				{
					next = Y;
					cursor.move(-next);
					last = cursor.get_pos() - next;
				}
				else // try to find nearest tile
				{
					// TODO use sf::IntRect for this?
					// TODO could probably refactor
					int xd {pos.x - 1};
					bool foundx {false};
					if (pos.y >= game.grid.get_min().y && pos.y <= game.grid.get_max().y)
						for (; xd >= game.grid.get_min().x; --xd)
							if (game.grid.get(xd, pos.y) != nullptr)
							{
								foundx = true;
								break;
							}

					int yd {pos.y - 1};
					bool foundy = false;
					if (pos.x >= game.grid.get_min().x && pos.x <= game.grid.get_max().x)
						for (; yd >= game.grid.get_min().y; --yd)
							if (game.grid.get(pos.x, yd) != nullptr)
							{
								foundy = true;
								break;
							}

					if (foundx)
					{
						if (!foundy || pos.x - xd <= pos.y - yd)
							next = X;
						else
							next = Y;
					}
					else if (foundy)
						next = Y;
					else // just do something
						next = X;

					cursor.move(-next);
					last = cursor.get_pos() - next;
				}
			}
			else // not near last character, position not empty, try to set autoadvance for next time
			{
				if (game.grid.get(pos - X) != nullptr)
				{
					next = X;
					last = cursor.get_pos() - next;
				}
				else if (game.grid.get(pos - Y) != nullptr)
				{
					next = Y;
					last = cursor.get_pos() - next;
				}
			}

			Tile* tile {game.grid.remove(cursor.get_pos())};
			if (tile != nullptr)
				game.hand.add_tile(tile);
		}

		// if letter key
		char ch;
		if (typer.get_ch(&ch))
		{
			bool placed {false};

			// if space is empty or has a different letter
			if (game.grid.get(cursor.get_pos()) == nullptr || game.grid.get(cursor.get_pos())->ch() != ch)
			{
				if (game.hand.has_any(ch))
				{
					Tile* tile {game.grid.swap(cursor.get_pos(), game.hand.remove_tile(ch))};

					if (tile != nullptr)
						game.hand.add_tile(tile);
					placed = true;
				}
			}
			else // space already has the letter to be placed
				placed = true;

			// if we placed a letter, try to autoadvance
			// TODO make autoadvancing smarter when not near last
			if (placed)
			{
				next = {0, 0};
				if (cursor.get_pos() == last + X)
					next.x = 1;
				else if (cursor.get_pos() == last + Y)
					next.y = 1;
				else if (game.grid.get(cursor.get_pos() - X) != nullptr)
					next.x = 1;
				else if (game.grid.get(cursor.get_pos() - Y) != nullptr)
					next.y = 1;
				last = cursor.get_pos();
				cursor.move(next);
			}
			else
			{
				stringstream letter;
				letter << ch;
				game.messages.add("You are out of " + letter.str() + "s!", Message::Severity::HIGH);
			}
		}

		// frame-time-dependent stuff
		float time {clock.getElapsedTime().asSeconds()};
		clock.restart();

		game.messages.age(time);

		bool keep_cursor_on_screen {false};

		if (controls["center"])
		{
			grid_view.setCenter(game.grid.get_center());
			keep_cursor_on_screen = true;
		}

		// zoom with mouse wheel
		if (state.wheel_delta < 0 || (state.wheel_delta > 0 && state.zoom > 1))
		{
			sf::Vector2f before {(state.pos[0] * gsize.x) / wsize.x + center.x - gsize.x / 2, (state.pos[1] * gsize.y) / wsize.y + center.y - gsize.y / 2};
			grid_view.zoom(1 - state.wheel_delta * time * 2);
			gsize = grid_view.getSize();
			sf::Vector2f after {(state.pos[0] * gsize.x) / wsize.x + center.x - gsize.x / 2, (state.pos[1] * gsize.y) / wsize.y + center.y - gsize.y / 2};
			grid_view.move(before - after);

			state.wheel_delta = 0;
			keep_cursor_on_screen = true;

		}

		if (keep_cursor_on_screen)
		{
			// move cursor if zooming in moves it off screen
			center = grid_view.getCenter();
			gsize = grid_view.getSize();
			sf::Vector2f spos = cursor.get_center();
			while (spos.x - center.x > gsize.x / 4)
			{
				cursor.move(-X);
				spos.x -= PPB;
			}

			while (spos.x - center.x < gsize.x / -4)
			{
				cursor.move(X);
				spos.x += PPB;
			}

			while (spos.y - center.y > gsize.y / 4)
			{
				cursor.move(-Y);
				spos.y -= PPB;
			}

			while (spos.y - center.y < gsize.y / -4)
			{
				cursor.move(Y);
				spos.y += PPB;
			}
		}
		int zoom {0};
		if (controls["zoom_in"])
			zoom += -1;
		if (controls["zoom_out"])
			zoom += 1;
		if (controls["zoom_in_fast"])
			zoom += -2;
		if (controls["zoom_out_fast"])
			zoom += 2;
		grid_view.zoom(1 + zoom * time);

		sf::Vector2i delta {0, 0};
		if (controls["left"])
			delta += -X;
		if (controls["right"])
			delta += X;
		if (controls["up"])
			delta += -Y;
		if (controls["down"])
			delta += Y;
		if (controls["left_fast"])
			delta += -X * 2;
		if (controls["right_fast"])
			delta += X * 2;
		if (controls["up_fast"])
			delta += -Y * 2;
		if (controls["down_fast"])
			delta += Y * 2;
		cursor.move(delta);

		// these might have changed due to zooming
		center = grid_view.getCenter();
		gsize = grid_view.getSize();
		sf::Vector2f spos = cursor.get_center();
		// TODO refactor
		// measure difference from a box in the center of the screen
		sf::Vector2f diff {(std::abs(spos.x - center.x) > gsize.x / 4 ? spos.x - center.x - (spos.x >= center.x ? gsize.x / 4 : gsize.x / -4) : 0), (std::abs(spos.y - center.y) > gsize.y / 4 ? spos.y  - center.y - (spos.y >= center.y ? gsize.y / 4 : gsize.y / -4) : 0)};
		// TODO is there a better movement function?
		diff /= (float)(1.0 + time * 400);
		grid_view.move(diff.x, diff.y);

		// don't allow zooming in past 1x
		if (gsize.x < wsize.x || gsize.y < wsize.y)
			grid_view.setSize(wsize.x, wsize.y);
		state.zoom = grid_view.getSize().x / wsize.x;

		cursor.set_zoom(state.zoom);
		mcursor.set_zoom(state.zoom);
		selection.set_zoom(state.zoom);

		// animate tiles
		game.grid.step(time);

		if (controls["scramble_tiles"])
			game.hand.set_scrambled();
		if (controls["sort_tiles"])
			game.hand.set_sorted();
		if (controls["count_tiles"])
			game.hand.set_counts();
		if (controls["stack_tiles"])
			game.hand.set_stacked();

		if (controls["menu"])
		{
			current.open();

			// insert menu after window event input reader
			for (auto it = input_readers.begin(); it != input_readers.end(); ++it)
			{
				if (*it == &win_events)
				{
					input_readers.insert(++it, &current);
					break;
				}
			}
		}

		// draw
		window.clear(background);

		window.setView(grid_view);
		game.grid.draw_on(window);
		if (game.selecting || game.selected)
			selection.draw_on(window);
		if (game.buffer != nullptr)
			game.buffer->draw_on(window);
		cursor.draw_on(window);
		mcursor.draw_on(window);

		window.setView(gui_view);
		game.messages.draw_on(window);
		game.hand.draw_on(window);

		if (!current.is_finished())
			current.menu().draw_on(window);

		window.display();
	}

	// TODO save game if player hasn't won

	// TODO only do this if controls changed
	controls.write_to_file("config.yaml");

	return 0;
}

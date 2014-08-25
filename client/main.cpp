#include "client.hpp"

// embedded resources
#include "icon.hpp"

// TODO sound

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
	State& state;
	MenuSystem& system;
	Game** game;
public:
	WindowEvents(State& s, MenuSystem& sys, Game** g)
		: state(s), system(sys), game {g}
	{
	}

	virtual bool process_event(sf::Event& event)
	{
		if (event.type == sf::Event::Closed)
		{
			state.window->close();

			finished = true;
			return false;
		}
		else if (event.type == sf::Event::Resized)
		{
			gui_view.setSize(event.size.width, event.size.height);
			gui_view.setCenter(event.size.width / 2.0, event.size.height / 2.0);
			state.grid_view->setSize(event.size.width, event.size.height);
			state.grid_view->zoom(state.zoom);
			system.menu().update_position();

			if (*game != nullptr)
				(*game)->get_hand().position_tiles();
		}
		return true;
	}
};

int main()
{
	// load resources
	if (!font.loadFromFile("DejaVuSans.ttf"))
	{
		cerr << "Couldn't find font DejaVuSans.ttf!\n";
		return 1;
	}

	SoundManager sound;
	// preload sounds
	sound.load("audio/split.wav");
	sound.load("audio/menu_move.wav");
	sound.load("audio/menu_open.wav");
	sound.load("audio/menu_select.wav");

	KeyControls controls;
	// TODO store last dictionary name, last resolution settings, etc.
	controls.load_from_file("config.yaml");

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

	// initialize menu system
	MenuSystem menu_system(sound);

	Menu main_menu {menu_system, nullptr, "BANANAGRAMS"};
	Entry solitaire   {"SOLITAIRE"};
	Entry multiplayer {"MULTIPLAYER"};
	Entry customize   {"CONTROLS"};
	Entry quit        {"QUIT"};
	main_menu.entry(&solitaire);
	main_menu.entry(&multiplayer);
	main_menu.entry(&customize);
	main_menu.entry(&quit);

	Menu sp_menu {menu_system, &main_menu, "SOLITAIRE"};
	Entry      start_sp   {"START GAME"};
	TextEntry  dict_entry {"DICTIONARY", PPB * 8, "dictionary.txt", "(default dictionary)"};
	MultiEntry multiplier {"BUNCH x", {"1/2", "1", "2", "3", "4", "Infinite"}, 1};
	sp_menu.entry(&start_sp);
	sp_menu.entry(&dict_entry);
	sp_menu.entry(&multiplier);

	// TODO add entry for disconnecting from game?
	Menu mp_menu {menu_system, &main_menu, "MULTIPLAYER"};
	Entry     start_mp {"JOIN"};
	TextEntry server   {"SERVER", PPB * 8, sf::IpAddress::getLocalAddress().toString() + ":" + std::to_string(default_server_port), "localhost"};
	TextEntry name     {"PLAYER NAME", PPB * 8, "Banana Brain", "Banana Brain"};
	mp_menu.entry(&server);
	mp_menu.entry(&name);
	mp_menu.entry(&start_mp);

	Menu control_menu {menu_system, &main_menu, "CONTROLS"};
	// TODO scrolling menus?
	// XXX this is a bit inefficient, but who cares?
	for (auto& command : controls.get_order())
		for (auto& pair : controls.get_binds())
			if (pair.second == command && controls.is_rebindable(pair.second))
				control_menu.entry(new ControlEntry(control_menu, controls, pair.second, pair.first));

	Menu quit_menu {menu_system, &main_menu, "Really quit?"};
	Entry quit_yes {"YES"};
	Entry quit_no  {"NO"};
	quit_menu.entry(&quit_yes);
	quit_menu.entry(&quit_no);

	menu_system.set_menu(main_menu);

	Game* game {nullptr};
	WindowEvents win_events {state, menu_system, &game};
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
		bool skip {false};
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
			if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Key::Escape)
				skip = true;
		}

		float elapsed {clock.getElapsedTime().asSeconds()};

		if (elapsed < 1)
			loading_text.setColor(sf::Color(255, 255, 255, 255 * elapsed));
		else if (elapsed > 2)
			loading_text.setColor(sf::Color(255, 255, 255, 255 - 255 * (elapsed - 2)));
		else
			loading_text.setColor(sf::Color::White);

		if (skip || elapsed > 3)
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

			// TODO make tiles prettier
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

	input_readers.push_back(&menu_system);

	// this will display too quickly to see if no game is loaded
	loading_text.setString("Loading saved game...");
	ltbounds = loading_text.getGlobalBounds();
	loading_text.setPosition(center.x - ltbounds.width / 2, center.y - ltbounds.height * 2.5);
	window.clear(background);
	window.draw(loading_text);
	window.display();

	// load saved game
	std::ifstream save_file("save.dat");
	if (save_file.is_open())
		game = new SingleplayerGame(sound, save_file);
	save_file.close();

	// stuff for game loop
	MouseControls mouse;
	input_readers.push_back(&mouse);

	input_readers.push_back(&controls);

	Typer single_typer(true);
	// indicates what single_typer is waiting for:
	// 0 nothing (not in input_readers)
	// 1 show
	// 2 dump
	unsigned int waiting_single = 0;

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

		if (!menu_system.is_finished())
		{
			if (solitaire.is_pending())
				menu_system.set_menu(sp_menu);
			if (multiplayer.is_pending())
				menu_system.set_menu(mp_menu);
			if (customize.is_pending())
				menu_system.set_menu(control_menu);
			if (quit.is_pending())
				menu_system.set_menu(quit_menu);

			if (start_sp.is_pending())
			{
				int mul {1};
				int div {1};

				// TODO nicer way to do this? decouple?
				int choice = multiplier.get_choice();
				if (choice == 0)
					div = 2;
				else if (choice == (int)multiplier.get_num_choices() - 1)
					div = 0; // infinite bunch
				else
					mul = choice;

				if (game != nullptr)
					delete game;

				// TODO display loading text
				game = new SingleplayerGame(sound, dict_entry.get_string(), mul, div);
			}

			if (start_mp.is_pending())
			{
				// TODO process name string

				if (game != nullptr)
					delete game;

				game = new MultiplayerGame(sound, server.get_string(), name.get_string());
			}

			if (quit_yes.is_pending())
				window.close();

			if (quit_no.is_pending())
				menu_system.set_menu(main_menu);
		}

		if (game != nullptr)
		{
			if (mouse.was_moved())
				game->update_mouse_pos(window, grid_view, mouse.get_pos());

			if (mouse.was_pressed(0))
				game->select();

			if (game->is_selecting())
				game->resize_selection();

			if (mouse.was_released(0))
			{
				if (game->complete_selection() == 1 && controls["quick_place"])
					game->quick_place();
			}

			if (controls["ready"])
				game->ready(); // multiplayer only

			if (controls["cut"])
				game->cut();

			if (controls["flip"])
				game->flip_buffer();

			if (controls["paste"])
				game->paste();

			if (mouse.is_held(1))
				game->remove_at_mouse();

			if (controls["peel"])
				game->peel();

			// if backspace
			if (controls["remove"])
				game->remove();

			bool add_single_typer = false;

			if (controls["show"])
			{
				if (!waiting_single)
					add_single_typer = true;

				waiting_single = 1;

				game->prompt_show();
			}

			if (controls["dump"])
			{
				if (!waiting_single)
					add_single_typer = true;

				waiting_single = 2;

				game->prompt_dump();
			}

			if (add_single_typer)
			{
				single_typer.reset();

				// insert before normal typer
				for (auto it = input_readers.begin(); it != input_readers.end(); ++it)
				{
					if (*it == &typer)
					{
						input_readers.insert(it, &single_typer);
						break;
					}
				}
			}

			char ch;

			if (single_typer.get_ch(&ch))
			{
				if (waiting_single == 0)
				{
					// should never get here
					cerr << "Somehow got a single typed character when not waiting...\n";
				}
				else if (waiting_single == 1) // show
				{
					game->show(ch);
				}
				else if (waiting_single == 2) // dump
				{
					game->dump(ch);
				}
				else
				{
					// should never get here
					cerr << "Invalid waiting type value.\n";
				}

				waiting_single = 0;
			}

			// if letter key
			if (typer.get_ch(&ch))
				game->place(ch);

			// frame-time-dependent stuff
			float time {clock.getElapsedTime().asSeconds()};
			clock.restart();

			if (controls["center"])
			{
				grid_view.setCenter(game->get_grid_center());
				game->set_cursor_to_view();
			}

			// zoom with mouse wheel
			int wheel_delta = mouse.get_wheel_delta();
			if (wheel_delta < 0 || (wheel_delta > 0 && state.zoom > 1))
			{
				sf::Vector2i pos = mouse.get_pos();
				sf::Vector2f before {(pos.x * gsize.x) / wsize.x + center.x - gsize.x / 2, (pos.y * gsize.y) / wsize.y + center.y - gsize.y / 2};
				grid_view.zoom(1 - wheel_delta * time * 2);
				gsize = grid_view.getSize();
				sf::Vector2f after {(pos.x * gsize.x) / wsize.x + center.x - gsize.x / 2, (pos.y * gsize.y) / wsize.y + center.y - gsize.y / 2};
				grid_view.move(before - after);

				game->set_cursor_to_view();
			}

			game->update_cursor(grid_view);

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
			game->move_cursor(delta);

			// these might have changed due to zooming
			center = grid_view.getCenter();
			gsize = grid_view.getSize();
			sf::Vector2f spos = game->get_cursor_center();
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

			game->set_zoom(state.zoom);

			// animate tiles
			game->step(time);

			if (controls["scramble_tiles"])
				game->get_hand().set_scrambled();
			if (controls["sort_tiles"])
				game->get_hand().set_sorted();
			if (controls["count_tiles"])
				game->get_hand().set_counts();
			if (controls["stack_tiles"])
				game->get_hand().set_stacked();
		}

		if (controls["menu"])
		{
			menu_system.open();

			// insert menu after window event input reader
			for (auto it = input_readers.begin(); it != input_readers.end(); ++it)
			{
				if (*it == &win_events)
				{
					input_readers.insert(++it, &menu_system);
					break;
				}
			}
			// menu input reader blocks game from getting menu key release
			controls.reset("menu");

			sound.play("audio/menu_open.wav");
		}

		// draw
		window.clear(background);

		if (game != nullptr)
			game->draw_on(window, grid_view, gui_view);

		window.setView(gui_view);
		if (!menu_system.is_finished())
			menu_system.menu().draw_on(window);

		window.display();
	}

	// cleanup game
	if (game != nullptr)
	{
		// if singleplayer, save game (or delete save if we finished it)
		SingleplayerGame* sp = dynamic_cast<SingleplayerGame*>(game);
		if (sp != nullptr)
		{
			if (game->in_progress())
				sp->save("save.dat");
			else
				std::remove("save.dat");
		}

		delete game;
	}

	controls.write_to_file("config.yaml");

	return 0;
}

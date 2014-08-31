#include <iostream>

#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>

#include "client.hpp"
#include "constants.hpp"
#include "control.hpp"
#include "icon.hpp"
#include "input.hpp"
#include "menu.hpp"
#include "server.hpp"
#include "sound.hpp"

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
	bool got_close = false;
	bool got_resize = false;
public:
	inline bool closed() const
	{
		return got_close;
	}

	inline bool resized()
	{
		bool r = got_resize;
		got_resize = false;
		return r;
	}

	virtual bool process_event(sf::Event& event)
	{
		if (event.type == sf::Event::Closed)
			got_close = true;
		else if (event.type == sf::Event::Resized)
			got_resize = true;

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
	MenuSystem menu_system;

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
	TextEntry server_ip   {"SERVER", PPB * 8, sf::IpAddress::getLocalAddress().toString() + ":" + std::to_string(default_server_port), "localhost"};
	TextEntry name     {"PLAYER NAME", PPB * 8, "Banana Brain", "Banana Brain"};
	mp_menu.entry(&server_ip);
	mp_menu.entry(&name);
	mp_menu.entry(&start_mp);

	Menu control_menu {menu_system, &main_menu, "CONTROLS"};
	// TODO scrolling menus?
	// TODO can we decouple controls object from entries?
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

	Client* client {nullptr};
	Server* server {nullptr};
	WindowEvents win_events;
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
		// TODO handle winevents here?

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
			// TODO handle winevents here?

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
				{
					if (*r == &menu_system)
						sound.play("audio/menu_open.wav");
					r = input_readers.erase(r);
				}
				else
					++r;
				if (!cont)
					break;
			}
		}

		if (win_events.closed())
			window.close();

		if (win_events.resized())
		{
			gui_view.setSize(event.size.width, event.size.height);
			gui_view.setCenter(event.size.width / 2.0, event.size.height / 2.0);

			state.grid_view->setSize(event.size.width, event.size.height);
			state.grid_view->zoom(state.zoom);

			if (menu_system.menu() != nullptr)
				menu_system.menu()->update_position();

			if (client != nullptr)
				client->get_hand().position_tiles();
		}

		if (!menu_system.is_finished())
		{
			bool selected = false;
			if (solitaire.is_pending())
			{
				menu_system.set_menu(sp_menu);
				selected = true;
			}
			if (multiplayer.is_pending())
			{
				menu_system.set_menu(mp_menu);
				selected = true;
			}
			if (customize.is_pending())
			{
				menu_system.set_menu(control_menu);
				selected = true;
			}
			if (quit.is_pending())
			{
				menu_system.set_menu(quit_menu);
				selected = true;
			}

			if (start_sp.is_pending())
			{
				selected = true;

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

				delete server;
				delete client;

				// TODO display loading text
				// TODO decouple sound from games
				server = new Server(default_server_port, dict_entry.get_string(), mul, div, 1);
				client = new Client(sound, sf::IpAddress("127.0.0.1"), default_server_port, "");
				// TODO menu isn't getting cleared for next action
				menu_system.close();
			}

			if (start_mp.is_pending())
			{
				selected = true;

				delete server;
				delete client;

				std::string ip {server_ip.get_string()};
				unsigned short server_port = default_server_port;

				// process server string
				// TODO make this a little more robust...
				size_t port_p {server_ip.get_string().find(':')};
				if (port_p != std::string::npos)
				{
					std::stringstream port_s;
					port_s << server_ip.get_string().substr(port_p + 1);
					port_s >> server_port;

					ip = server_ip.get_string().substr(0, port_p);
				}

				// TODO process name string
				client = new Client(sound, sf::IpAddress(ip), server_port, name.get_string());
				menu_system.close();
			}

			if (quit_yes.is_pending())
			{
				selected = true;

				window.close();
				break;
			}

			if (quit_no.is_pending())
			{
				selected = true;
				menu_system.set_menu(main_menu);
			}

			if (selected)
				sound.play("audio/menu_select.wav");

			if (menu_system.selection_was_changed())
				sound.play("audio/menu_move.wav");
		}

		if (menu_system.menu_was_changed())
			sound.play("audio/menu_select.wav");

		if (client != nullptr)
		{
			if (mouse.was_moved())
				client->update_mouse_pos(window, grid_view, mouse.get_pos());

			if (mouse.was_pressed(0))
				client->select();

			if (client->is_selecting())
				client->resize_selection();

			if (mouse.was_released(0))
			{
				if (client->complete_selection() == 1 && controls["quick_place"])
					client->quick_place();
			}

			if (controls["ready"])
				client->ready(); // multiplayer only

			if (controls["cut"])
				client->cut();

			if (controls["flip"])
				client->flip_buffer();

			if (controls["paste"])
				client->paste();

			if (mouse.is_held(1))
				client->remove_at_mouse();

			if (controls["peel"])
				client->peel();

			// if backspace
			if (controls["remove"])
				client->remove();

			bool add_single_typer = false;

			if (controls["show"])
			{
				if (!waiting_single)
					add_single_typer = true;

				waiting_single = 1;

				client->prompt_show();
			}

			if (controls["dump"])
			{
				if (!waiting_single)
					add_single_typer = true;

				waiting_single = 2;

				client->prompt_dump();
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
					client->show(ch);
				}
				else if (waiting_single == 2) // dump
				{
					client->dump(ch);
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
				client->place(ch);

			// frame-time-dependent stuff
			float time {clock.getElapsedTime().asSeconds()};
			clock.restart();

			if (controls["center"])
			{
				grid_view.setCenter(client->get_grid_center());
				client->set_cursor_to_view();
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

				client->set_cursor_to_view();
			}

			client->update_cursor(grid_view);

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
			client->move_cursor(delta);

			// these might have changed due to zooming
			center = grid_view.getCenter();
			gsize = grid_view.getSize();
			sf::Vector2f spos = client->get_cursor_center();
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

			client->set_zoom(state.zoom);

			// animate tiles
			client->step(time);

			if (controls["scramble_tiles"])
				client->get_hand().set_scrambled();
			if (controls["sort_tiles"])
				client->get_hand().set_sorted();
			if (controls["count_tiles"])
				client->get_hand().set_counts();
			if (controls["stack_tiles"])
				client->get_hand().set_stacked();
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
			// menu input reader blocks client from getting menu key release
			controls.reset("menu");

			sound.play("audio/menu_open.wav");
		}

		// draw
		window.clear(background);

		if (client != nullptr)
			client->draw_on(window, grid_view, gui_view);

		window.setView(gui_view);
		if (!menu_system.is_finished() && menu_system.menu() != nullptr)
			menu_system.menu()->draw_on(window);

		window.display();
	}

	// cleanup client
	delete client;

	controls.write_to_file("config.yaml");

	return 0;
}

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
public:
	WindowEvents(State& s, MenuSystem& sys)
		: state(s), system(sys)
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

	KeyControls controls;
	// TODO store last dictionary name, last resolution settings, etc.
	controls.load_from_file("config.yaml");

	std::srand(std::time(nullptr));

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
	MenuSystem current;
	Menu main {current, nullptr, "BANANAGRAMS"};
	MenuEntry solitaire {"SOLITAIRE", current};
	MenuEntry multiplayer {"MULTIPLAYER", current};
	MenuEntry customize {"CONTROLS", current};
	MenuEntry quit {"QUIT", current};
	main.append_entry(&solitaire);
	main.append_entry(&multiplayer);
	main.append_entry(&customize);
	main.append_entry(&quit);

	// create solitaire menu
	Menu solitaire_opts {current, &main, "SOLITAIRE"};
	solitaire.submenu = &solitaire_opts;

	Game* game {nullptr};
	TextEntry dict_entry {"DICTIONARY", PPB * 8, "dictionary.txt"};
	MultiEntry multiplier {"BUNCH x", {"1/2", "1", "2", "3", "4"}, 1};
	SolitaireEntry start {"START GAME", current, dict_entry, multiplier, &game};

	solitaire_opts.append_entry(&start);
	solitaire_opts.append_entry(&dict_entry);
	solitaire_opts.append_entry(&multiplier);

	Menu multiplayer_menu {current, &main, "MULTIPLAYER"};
	multiplayer.submenu = &multiplayer_menu;

	std::string def_ip = sf::IpAddress::getLocalAddress().toString();
	std::string def_pt = std::to_string(default_port);
	TextEntry server {"SERVER", PPB * 8, def_ip + def_pt};
	TextEntry name {"PLAYER NAME", PPB * 8, "Banana Brain"};
	MultiplayerEntry join {"JOIN", current, server, name, &game};
	multiplayer_menu.append_entry(&server);
	multiplayer_menu.append_entry(&name);
	multiplayer_menu.append_entry(&join);

	// create control menu
	Menu control_opts {current, &main, "CONTROLS"};
	customize.submenu = &control_opts;

	// TODO scrolling menus?
	// TODO order these
	for (auto& pair : controls.get_binds())
		if (controls.is_rebindable(pair.second))
			control_opts.append_entry(new ControlEntry(control_opts, controls, pair.second, pair.first));

	// create quite menu
	Menu confirm_quit {current, &main, "Really quit?"};
	quit.submenu = &confirm_quit;
	QuitEntry yes {"YES", window};
	MenuEntry no {"NO", current, &main};
	confirm_quit.append_entry(&yes);
	confirm_quit.append_entry(&no);

	current.set_menu(main);

	WindowEvents win_events {state, current};
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

	input_readers.push_back(&current);

	loading_text.setString("Loading dictionary...");
	ltbounds = loading_text.getGlobalBounds();
	loading_text.setPosition(center.x - ltbounds.width / 2, center.y - ltbounds.height * 2.5);
	window.clear(background);
	window.draw(loading_text);
	window.display();

	// stuff for game loop
	MouseControls mouse;
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

			if (controls["cut"])
				game->cut();

			if (controls["flip"])
				game->flip_buffer();

			if (controls["paste"])
				game->paste();

			if (mouse.is_held(1))
				game->remove_at_mouse();

			if (controls["dump"])
				game->dump();

			if (controls["peel"])
				game->peel();

			// if backspace
			if (controls["remove"])
				game->remove();

			// if letter key
			char ch;
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

		if (game != nullptr)
			game->draw_on(window, grid_view, gui_view);

		window.setView(gui_view);
		if (!current.is_finished())
			current.menu().draw_on(window);

		window.display();
	}

	// TODO save game if player hasn't won
	if (game != nullptr)
		delete game;

	// TODO only do this if controls changed
	controls.write_to_file("config.yaml");

	return 0;
}

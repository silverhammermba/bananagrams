#include "bananagrams.hpp"

// embedded resources
#include "icon.hpp"

using std::cerr;
using std::endl;
using std::string;
using std::stringstream;
using std::vector;

// objects need globally
sf::RenderTexture tile_texture[26];
std::map<string, string> dictionary;

// class for handling game-related events
class Game : public InputReader
{
	State* state;
public:
	Game(State* s)
	{
		state = s;
	}

	virtual bool process_event(sf::Event& event)
	{
		if (event.type == sf::Event::Closed || (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape))
		{
			state->window->close();

			finished = true;
			return false;
		}
		else if (event.type == sf::Event::Resized)
		{
			state->gui_view->setSize(event.size.width, event.size.height);
			state->gui_view->setCenter(event.size.width / 2.0, event.size.height / 2.0);
			state->grid_view->setSize(event.size.width, event.size.height);
			state->grid_view->zoom(state->zoom);
		}
		return true;
	}
};

// TODO profile
int main()
{
	// load resources
	sf::Font font;
	if (!font.loadFromFile("/usr/share/fonts/TTF/DejaVuSans.ttf"))
	{
		cerr << "Couldn't find font /usr/share/fonts/TTF/DejaVuSans.ttf!\n";
		return 1;
	}

	// TODO validate somehow
	std::ifstream words("dictionary.txt");
	if (!words.is_open())
	{
		cerr << "Couldn't find dictionary.txt!\n";
		return 1;
	}

	KeyControls controls;
	controls.load_from_file("config.yaml");

	std::srand((unsigned int)std::time(nullptr));

	Grid grid;

	unsigned int letter_count[26] =
	{
		13,
		3,
		3,
		6,
		18,
		3,
		4,
		3,
		12,
		2,
		2,
		5,
		3,
		8,
		11,
		3,
		2,
		9,
		6,
		9,
		6,
		3,
		3,
		2,
		3,
		2
	};

	sf::RenderWindow window(sf::VideoMode(1280, 720), "Bananagrams");
	window.setIcon(32, 32, icon);
	window.setVerticalSyncEnabled(true);

	sf::View gui_view = window.getDefaultView();
	sf::View grid_view = window.getDefaultView();
	grid_view.setCenter(PPB / 2.0, PPB / 2.0);

	vector<InputReader*> input_readers;

	State state;
	state.window = &window;
	state.gui_view = &gui_view;
	state.grid_view = &grid_view;
	state.zoom = 1;

	Game game(&state);
	input_readers.push_back(&game);

	sf::Color background(22, 22, 22);

	sf::Clock clock;

	sf::Text loading_text("for Monica", font, 30);
	loading_text.setColor(sf::Color(255, 255, 255, 0));
	auto bounds = loading_text.getGlobalBounds();
	auto center = gui_view.getCenter();
	loading_text.setPosition(center.x + bounds.width / -2, center.y + bounds.height / -2);

	// dedication
	// TODO make window resizing/quitting work here
	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			for (auto r = input_readers.begin(); r != input_readers.end();)
			{
				bool cont = (*r)->process_event(event);
				if ((*r)->is_finished())
					r = input_readers.erase(r);
				else
					r++;
				if (!cont)
					break;
			}
		}

		float elapsed = clock.getElapsedTime().asSeconds();

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

	loading_text.setColor(sf::Color::White);

	loading_text.setString("Loading dictionary...");
	loading_text.setPosition(center.x + loading_text.getGlobalBounds().width / -2, center.y - 90);
	window.clear(background);
	window.draw(loading_text);
	window.display();

	string line;
	while (std::getline(words, line))
	{
		auto pos = line.find_first_of(' ');
		if (pos == string::npos)
			dictionary[line] = "";
		else
			dictionary[line.substr(0, pos)] = line.substr(pos + 1, string::npos);
	}
	words.close();

	stringstream foo;
	foo << dictionary.size();
	loading_text.setString(foo.str() + " words loaded.");
	loading_text.setPosition(center.x + loading_text.getGlobalBounds().width / -2, center.y - 90);

	std::list<Tile*> bunch;
	Hand hand(&gui_view, font);

	{
		// for generating tiles
		float padding = PPB / 2.0;
		char load_char = 'A';
		float miny = 0;
		float height = 0;
		bool done_y = false;
		vector<sf::Sprite> loaded;

		// generate tile textures
		while (window.isOpen())
		{
			sf::Event event;
			while (window.pollEvent(event))
			{
				for (auto r = input_readers.begin(); r != input_readers.end();)
				{
					bool cont = (*r)->process_event(event);
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
			sf::Text letter(str.str(), font, (PPB * 2) / 3.0);
			letter.setColor(sf::Color::Black);

			// center
			auto bounds = letter.getGlobalBounds();
			float minx = bounds.left;
			float width = bounds.width;
			if (!done_y)
			{
				miny = bounds.top;
				height = bounds.height;
			}
			done_y = true;
			letter.setPosition((PPB - width) / 2.0 - minx, (PPB - height) / 2.0 - miny);

			float diam = PPB / 4.0;
			float rad = PPB / 8.0;
			// draw tile
			tile_texture[load_char - 'A'].clear(sf::Color(0, 0, 0, 0));

			sf::RectangleShape rect;
			rect.setFillColor(sf::Color(255, 255, 175));

			rect.setSize(sf::Vector2f(PPB - diam, PPB));
			rect.setPosition(rad, 0);
			tile_texture[load_char - 'A'].draw(rect);
			rect.setSize(sf::Vector2f(PPB, PPB - diam));
			rect.setPosition(0, rad);
			tile_texture[load_char - 'A'].draw(rect);

			sf::CircleShape circle(rad);
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

			// create tiles for the bunch
			for (unsigned int i = 0; i < letter_count[load_char - 'A']; i++)
			{
				auto it = bunch.begin();
				auto pos = std::rand() % (bunch.size() + 1);
				for (unsigned int i = 0; i != pos && it != bunch.end(); it++, i++);
				bunch.insert(it, new Tile(load_char));
			}

			// display generated texture
			loaded.push_back(sf::Sprite(tile_texture[load_char - 'A'].getTexture()));

			window.clear(background);
			window.draw(loading_text);
			unsigned int i = 0;
			float vwidth = gui_view.getSize().x;
			auto vcenter = gui_view.getCenter();
			for (auto& sprite : loaded)
			{
				sprite.setPosition((i * (vwidth - 2 * padding - PPB)) / 25.0 + vcenter.x - vwidth / 2 + padding, vcenter.y + PPB / -2.0);
				window.draw(sprite);
				++i;
			}
			window.display();

			load_char++;
			// if we're done
			if(load_char > 'Z')
			{
				// take tiles from the bunch for player
				for (unsigned int i = 0; i < 21; i++)
				{
					Tile* tile = bunch.back();
					bunch.pop_back();
					hand.add_tile(tile);
				}

				break;
			}
		}
	}

	// stuff for game loop
	MessageQ messages(font);

	Cursor cursor(sf::Vector2u(1, 1), PPB / 16.0, sf::Color(0, 0, 0, 0), sf::Color(0, 200, 0));
	Cursor mcursor(sf::Vector2u(1, 1), PPB / 16.0, sf::Color(0, 0, 0, 0), sf::Color(0, 200, 0, 80));

	sf::Vector2i last(-1, 0);
	sf::Vector2i next(0, 0);

	CutBuffer* buffer = nullptr;
	bool selected = false;
	bool selecting = false;
	// mouse press and release positions
	sf::Vector2i sel1;
	sf::Vector2i sel2;
	Cursor selection(sf::Vector2u(1, 1), 1, sf::Color(255, 255, 255, 25), sf::Color::White);

	// mouse
	state.pos[0] = 0;
	state.pos[1] = 0;
	state.update = false;
	state.mremove = false;
	state.wheel_delta = 0;
	state.start_selection = false;
	state.end_selection = false;

	MouseControls mouse(&state);
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
				bool cont = (*r)->process_event(event);
				if ((*r)->is_finished())
					r = input_readers.erase(r);
				else
					r++;
				if (!cont)
					break;
			}
		}

		// mouse moved
		if (state.update)
		{
			// TODO refactor
			// update mouse cursor position
			mcursor.set_pos(sf::Vector2i(std::floor(((state.pos[0] * gsize.x) / wsize.x + center.x - (gsize.x / 2)) / PPB), std::floor(((state.pos[1] * gsize.y) / wsize.y + center.y - (gsize.y / 2)) / PPB)));

			if (buffer != nullptr)
				buffer->set_pos(mcursor.get_pos());

			state.update = false;
		}

		// left click
		if (state.start_selection)
		{
			selecting = true;
			selected = false;
			sel1 = mcursor.get_pos();
			state.start_selection = false;
		}

		// if left click held down
		if (selecting)
		{
			sel2 = mcursor.get_pos();
			// update selection rect
			selection.set_size(sf::Vector2u(std::abs(sel1.x - sel2.x) + 1, std::abs(sel1.y - sel2.y) + 1));
			selection.set_pos(sf::Vector2i(std::min(sel1.x, sel2.x), std::min(sel1.y, sel2.y)));
		}

		// if left click release
		if (state.end_selection)
		{
			selecting = false;
			selected = true;

			// if the selection was only 1 square, do something else
			if (selection.get_size() == sf::Vector2u(1, 1))
			{
				selected = false;
				// move cursor to mouse cursor
				cursor.set_pos(mcursor.get_pos());

				if (controls["quick_place"])
				{
					// look for remaining tiles
					char last = 'A' - 1;
					for (char ch = 'A'; ch <= 'Z'; ch++)
					{
						if (hand.has_any(ch))
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
						messages.add("You do not have any tiles.", MessageQ::LOW);
					else if (last > 'Z')
						messages.add("You have too many letters to place using the mouse.", MessageQ::HIGH);
					else
					{
						Tile* tile = grid.swap(cursor.get_pos(), hand.remove_tile(last));

						if (tile != nullptr)
							hand.add_tile(tile);
					}
				}
			}
			state.end_selection = false;
		}

		if (controls["cut"])
		{
			if (buffer == nullptr)
			{
				if (selected)
				{
					buffer = new CutBuffer(grid, std::min(sel1.x, sel2.x), std::min(sel1.y, sel2.y), selection.get_size());

					if (buffer->is_empty())
					{
						messages.add("Nothing selected.", MessageQ::LOW);
						delete buffer;
						buffer = nullptr;
					}
				}
				else
					messages.add("Nothing selected.", MessageQ::LOW);

				selected = false;
			}
			else
			{
				buffer->clear(hand);
				delete buffer;
				buffer = nullptr;
				messages.add("Added cut tiles back to your hand.", MessageQ::LOW);
			}
		}

		if (controls["flip"])
		{
			if (buffer != nullptr)
				buffer->transpose();
		}

		if (controls["paste"])
		{
			if (buffer != nullptr)
			{
				buffer->paste(grid, hand);
				delete buffer;
				buffer = nullptr;
			}
			else
				messages.add("Cannot paste: no tiles were cut.", MessageQ::LOW);
		}

		if (state.mremove)
		{
			// remove tile
			Tile* tile = grid.remove(mcursor.get_pos());
			if (tile != nullptr)
				hand.add_tile(tile);
		}

		if (controls["dump"])
		{
			if (bunch.size() >= 3)
			{
				auto dumped = grid.remove(cursor.get_pos());
				if (dumped == nullptr)
					messages.add("You need to select a tile to dump.", MessageQ::LOW);
				else
				{
					// take three
					for (unsigned int i = 0; i < 3; i++)
					{
						Tile* tile = bunch.back();
						bunch.pop_back();
						hand.add_tile(tile);
					}

					// add tile to bunch in random position
					auto it = bunch.begin();
					auto j = std::rand() % (bunch.size() + 1);
					for (unsigned int i = 0; i != j && it != bunch.end(); it++, i++);
					bunch.insert(it, dumped);
				}
			}
			else
				messages.add("There are not enough tiles left to dump!", MessageQ::HIGH);

		}

		if (controls["peel"])
		{
			bool spent = true;
			for (char ch = 'A'; ch <= 'Z'; ch++)
				if (hand.has_any(ch))
				{
					spent = false;
					break;
				}
			if (spent)
			{
				vector<string> mess;
				if (grid.is_valid(mess))
				{
					if (bunch.size() > 0)
					{
						Tile* tile = bunch.back();
						bunch.pop_back();
						hand.add_tile(tile);
						for (auto message : mess)
							messages.add(message, MessageQ::LOW);
					}
					else
						messages.add("You win!", MessageQ::LOW);
				}
				else
					for (auto message : mess)
						messages.add(message, MessageQ::HIGH);
			}
			else
				messages.add("You have not used all of your letters.", MessageQ::HIGH);
		}

		// if backspace
		if (controls["remove"])
		{

			// TODO DRY off autoadvancing
			// if the cursor is ahead of the last added character, autoadvance
			if (cursor.get_pos() == last + next)
			{
				cursor.set_pos(last);
				last -= next;
			}
			// else if you are not near the last character and the space is empty, try to autoadvance
			else if (grid.get(cursor.get_pos()) == nullptr)
			{
				if (grid.get(cursor.get_pos() - X) != nullptr)
				{
					next = X;
					cursor.move(-next);
					last = cursor.get_pos() - next;
				}
				else if (grid.get(cursor.get_pos() - Y) != nullptr)
				{
					next = Y;
					cursor.move(-next);
					last = cursor.get_pos() - next;
				}
			}
			else // not near last character, position not empty, try to set autoadvance for next time
			{
				if (grid.get(cursor.get_pos() - X) != nullptr)
				{
					next = X;
					last = cursor.get_pos() - next;
				}
				else if (grid.get(cursor.get_pos() - Y) != nullptr)
				{
					next = Y;
					last = cursor.get_pos() - next;
				}
			}

			auto tile = grid.remove(cursor.get_pos());
			if (tile != nullptr)
				hand.add_tile(tile);
		}

		// if letter key
		char ch;
		if (typer.get_ch(&ch))
		{
			bool placed = false;

			// if space is empty or has a different letter
			if (grid.get(cursor.get_pos()) == nullptr || grid.get(cursor.get_pos())->ch() != ch)
			{
				if (hand.has_any(ch))
				{
					Tile* tile = grid.swap(cursor.get_pos(), hand.remove_tile(ch));

					if (tile != nullptr)
						hand.add_tile(tile);
					placed = true;
				}
			}
			else // space already has the letter to be placed
				placed = true;

			// if we placed a letter, try to autoadvance
			if (placed)
			{
				next = {0, 0};
				if (cursor.get_pos() == last + X)
					next.x = 1;
				else if (cursor.get_pos() == last + Y)
					next.y = 1;
				else if (grid.get(cursor.get_pos() - X) != nullptr)
					next.x = 1;
				else if (grid.get(cursor.get_pos() - Y) != nullptr)
					next.y = 1;
				last = cursor.get_pos();
				cursor.move(next);
			}
			else
			{
				stringstream letter;
				letter << ch;
				messages.add("You are out of " + letter.str() + "s!", MessageQ::HIGH);
			}
		}

		// frame-time-dependent stuff
		float time = clock.getElapsedTime().asSeconds();
		clock.restart();

		messages.age(time);

		bool keep_cursor_on_screen = false;

		if (controls["center"])
		{
			grid_view.setCenter(grid.get_center());
			keep_cursor_on_screen = true;
		}

		// zoom with mouse wheel
		if (state.wheel_delta < 0 || (state.wheel_delta > 0 && state.zoom > 1))
		{
			sf::Vector2f before((state.pos[0] * gsize.x) / wsize.x + center.x - gsize.x / 2, (state.pos[1] * gsize.y) / wsize.y + center.y - gsize.y / 2);
			grid_view.zoom(1 - state.wheel_delta * time * 2);
			gsize = grid_view.getSize();
			sf::Vector2f after((state.pos[0] * gsize.x) / wsize.x + center.x - gsize.x / 2, (state.pos[1] * gsize.y) / wsize.y + center.y - gsize.y / 2);
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
		int zoom = 0;
		if (controls["zoom_in"])
			zoom += -1;
		if (controls["zoom_out"])
			zoom += 1;
		if (controls["zoom_in_fast"])
			zoom += -2;
		if (controls["zoom_out_fast"])
			zoom += 2;
		grid_view.zoom(1 + zoom * time);

		sf::Vector2i delta(0, 0);
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
		sf::Vector2f diff((std::abs(spos.x - center.x) > gsize.x / 4 ? spos.x - center.x - (spos.x >= center.x ? gsize.x / 4 : gsize.x / -4) : 0), (std::abs(spos.y - center.y) > gsize.y / 4 ? spos.y  - center.y - (spos.y >= center.y ? gsize.y / 4 : gsize.y / -4) : 0));
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
		grid.step(time);

		if (controls["scramble_tiles"])
			hand.set_scrambled();
		if (controls["sort_tiles"])
			hand.set_sorted();
		if (controls["count_tiles"])
			hand.set_counts();
		if (controls["stack_tiles"])
			hand.set_stacked();

		// draw
		window.clear(background);

		window.setView(grid_view);
		grid.draw_on(window);
		if (selecting || selected)
			selection.draw_on(window);
		if (buffer != nullptr)
			buffer->draw_on(window);
		cursor.draw_on(window);
		mcursor.draw_on(window);

		window.setView(gui_view);
		messages.draw_on(window);
		hand.draw_on(window);

		window.display();
	}

	// delete unused tiles
	for (auto tile: bunch)
		delete tile;

	// TODO only do this if controls changed
	controls.write_to_file("config.yaml");

	return 0;
}

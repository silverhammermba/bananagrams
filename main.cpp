#include <cmath>
#include <iostream>
#include <fstream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <SFML/Graphics.hpp>

using std::cerr;
using std::endl;

#define PPB 64

sf::RenderTexture tile_texture[26];

class InputReader
{
protected:
	bool finished;
public:
	InputReader() { finished = false; }
	virtual ~InputReader() {}

	bool is_finished() const { return finished; }
	virtual bool process_event(const sf::Event& event) = 0;
};

// class for handling game-related events
class Game : public InputReader
{
	sf::RenderWindow* window;
public:

	Game(sf::RenderWindow* win)
	{
		window = win;
	}

	virtual bool process_event(const sf::Event& event)
	{
		if (event.type == sf::Event::Closed || (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape))
		{
			window->close();

			finished = true;
			return false;
		}
		return true;
	}
};

class SimpleControls : public InputReader
{
	int* delta;
	char* ch;
	bool* zoom_key;
	bool* sprint_key;
	bool* backspace;
public:
	SimpleControls(int* d, char* c, bool* z, bool* s, bool* b)
	{
		delta = d;
		ch = c;
		zoom_key = z;
		sprint_key = s;
		backspace = b;
	}

	virtual bool process_event(const sf::Event& event)
	{
		if (event.type == sf::Event::KeyPressed)
		{
			switch (event.key.code)
			{
				case sf::Keyboard::Left:
					delta[0] = -1;
					break;
				case sf::Keyboard::Right:
					delta[0] = 1;
					break;
				case sf::Keyboard::Up:
					delta[1] = -1;
					break;
				case sf::Keyboard::Down:
					delta[1] = 1;
					break;
				case sf::Keyboard::LControl:
				case sf::Keyboard::RControl:
					*zoom_key = true;
					break;
				case sf::Keyboard::LShift:
				case sf::Keyboard::RShift:
					*sprint_key = true;
					break;
				case sf::Keyboard::BackSpace:
					*backspace = true;
					break;
				default:
					break;
			}
			if (sf::Keyboard::A <= event.key.code && event.key.code <= sf::Keyboard::Z)
				*ch = event.key.code - sf::Keyboard::A + 'A';
		}
		else if (event.type == sf::Event::KeyReleased)
		{
			switch (event.key.code)
			{
				case sf::Keyboard::Left:
				case sf::Keyboard::Right:
					delta[0] = 0;
					break;
				case sf::Keyboard::Up:
				case sf::Keyboard::Down:
					delta[1] = 0;
					break;
				case sf::Keyboard::LControl:
				case sf::Keyboard::RControl:
					*zoom_key = false;
					break;
				case sf::Keyboard::LShift:
				case sf::Keyboard::RShift:
					*sprint_key = false;
					break;
				default:
					break;
			}
		}
		return true;
	}
};

class VimControls : public InputReader
{
	bool shift = false;
	int* delta;
	char* ch;
	bool* zoom_key;
	bool* backspace;
public:
	VimControls(int* d, char* c, bool* z, bool* b)
	{
		delta = d;
		ch = c;
		zoom_key = z;
		backspace = b;
	}

	virtual bool process_event(const sf::Event& event)
	{
		if (event.type == sf::Event::KeyPressed)
		{
			// return to skip processing letter key as insert
			switch (event.key.code)
			{
				// directions
				case sf::Keyboard::H:
					if (!shift)
						break;
				case sf::Keyboard::Left:
					delta[0] = -1;
					return true;
				case sf::Keyboard::L:
					if (!shift)
						break;
				case sf::Keyboard::Right:
					delta[0] = 1;
					return true;
				case sf::Keyboard::K:
					if (!(shift || *zoom_key))
						break;
				case sf::Keyboard::Up:
					delta[1] = -1;
					return true;
				case sf::Keyboard::J:
					if (!(shift || *zoom_key))
						break;
				case sf::Keyboard::Down:
					delta[1] = 1;
					return true;
				// modifier keys
				case sf::Keyboard::LControl:
				case sf::Keyboard::RControl:
					*zoom_key = true;
					break;
				case sf::Keyboard::LShift:
				case sf::Keyboard::RShift:
					shift = true;
					break;
				case sf::Keyboard::X:
					if (!shift)
						break;
				case sf::Keyboard::BackSpace:
					*backspace = true;
					return true;
				default:
					break;
			}
			if (sf::Keyboard::A <= event.key.code && event.key.code <= sf::Keyboard::Z)
				*ch = event.key.code - sf::Keyboard::A + 'A';
		}
		else if (event.type == sf::Event::KeyReleased)
		{
			switch (event.key.code)
			{
				case sf::Keyboard::H:
				case sf::Keyboard::L:
				case sf::Keyboard::Left:
				case sf::Keyboard::Right:
					delta[0] = 0;
					break;
				case sf::Keyboard::J:
				case sf::Keyboard::K:
				case sf::Keyboard::Up:
				case sf::Keyboard::Down:
					delta[1] = 0;
					break;
				case sf::Keyboard::LControl:
				case sf::Keyboard::RControl:
					*zoom_key = false;
					if (!shift)
						delta[1] = 0;
					break;
				case sf::Keyboard::LShift:
				case sf::Keyboard::RShift:
					shift = false;
					delta[0] = 0;
					delta[1] = 0;
					break;
				default:
					break;
			}
		}
		return true;
	}
};

class Tile
{
	char character;
	sf::Sprite sprite;
public:
	Tile(char ch) :
		sprite(tile_texture[ch - 'A'].getTexture())
	{
		character = ch;
	}

	char ch() const
	{
		return character;
	}

	void set_pos(int x, int y)
	{
		sprite.setPosition(x * PPB, y * PPB);
	}

	void draw_on(sf::RenderWindow & window) const
	{
		window.draw(sprite);
	}
};

class Grid
{
	std::vector<Tile*> grid;

	inline unsigned int bijection(unsigned x, unsigned int y) const
	{
		return ((x + y) * (x + y + 1)) / 2 + x;
	}

	unsigned int convert(int x, int y) const
	{
		if (x >= 0 && y >= 0)
			return 4 * bijection(x, y);
		if (x < 0 && y >= 0)
			return 4 * bijection(-x - 1, y) + 1;
		if (x >= 0 && y < 0)
			return 4 * bijection(x, -y - 1) + 2;
		else
			return 4 * bijection(-x - 1, -y - 1) + 3;
	}
public:
	Grid() : grid()
	{
	}

	~Grid()
	{
		for (auto tile : grid)
			if (tile != nullptr)
				delete tile;
	}

	Tile* get(int x, int y) const
	{
		unsigned int n = convert(x, y);
		if (n >= grid.size())
			return nullptr;
		return grid[n];
	}

	Tile* remove(int x, int y)
	{
		unsigned int n = convert(x, y);
		if (n < grid.size())
		{
			Tile* tile = grid[n];
			if (tile == nullptr)
				return nullptr;

			grid[n] = nullptr;
			for (n = grid.size(); n --> 0;)
				if (grid[n] != nullptr)
					break;
			// TODO won't ever shrink to 0
			grid.resize(n + 1, nullptr);
			return tile;
		}
		return nullptr;
	}

	Tile* swap(int x, int y, Tile* tile)
	{
		if (tile == nullptr)
			throw std::runtime_error("attempt to place NULL tile");
		unsigned int n = convert(x, y);
		if (n >= grid.size())
			// TODO catch failure?
			grid.resize(n + 1, nullptr);
		Tile* swp = grid[n];
		tile->set_pos(x, y);
		grid[n] = tile;
		return swp;
	}

	void draw_on(sf::RenderWindow& window) const
	{
		for (auto tile : grid)
			if (tile != nullptr)
				tile->draw_on(window);
	}
};

int main()
{
	sf::Font font;
	font.loadFromFile("/usr/share/fonts/TTF/FreeSans.ttf");
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

	sf::Clock clock;

	// load word list
	cerr << "Reading dictionary... \x1b[s";
	std::map<std::string, std::string> dictionary;
	{
		std::ifstream words("dictionary.txt");

		std::string line;
		while (std::getline(words, line))
		{
			auto pos = line.find_first_of(' ');
			if (pos == std::string::npos)
			{
				dictionary[line] = "";
				if (std::rand() % 8 == 0)
					cerr << "\x1b[u\x1b[K" << line;
			}
			else
			{
				dictionary[line.substr(0, pos)] = line.substr(pos + 1, std::string::npos);
				if (std::rand() % 8 == 0)
					cerr << "\x1b[u\x1b[K" << line.substr(0, pos);
			}
		}
	}
	float time = clock.getElapsedTime().asSeconds();
	cerr << "\x1b[u\x1b[K" << dictionary.size() << " words loaded (" << time << ")\n";

	// create textures and tiles
	cerr << "Generating textures... ";
	std::vector<Tile*> tiles[26];

	clock.restart();
	{
		unsigned int miny = PPB;
		unsigned int maxy = 0;
		bool done_y = false;
		for (char ch = 'A'; ch <= 'Z'; ch++)
		{
			// TODO antialiase these
			// TODO error check
			// TODO memory leak here?
			tile_texture[ch - 'A'].create(PPB, PPB);

			// draw character for centering
			std::stringstream string;
			string << ch;
			sf::Text letter(string.str(), font, (PPB * 2) / 3.0);
			letter.setColor(sf::Color::Black);
			tile_texture[ch - 'A'].clear(sf::Color::Red);
			tile_texture[ch - 'A'].draw(letter);
			tile_texture[ch - 'A'].display();

			// center character
			unsigned int minx = PPB;
			unsigned int maxx = 0;
			auto image = tile_texture[ch - 'A'].getTexture().copyToImage();
			auto size = image.getSize();
			for (unsigned int x = 0; x < size.x; x++)
				for (unsigned int y = 0; y < size.y; y++)
					if (image.getPixel(x, y) != sf::Color::Red)
					{
						if (x < minx)
							minx = x;
						if (x > maxx)
							maxx = x;
						if (!done_y)
						{
							if (y < miny)
								miny = y;
							if (y > maxy)
								maxy = y;
						}
					}
			done_y = true;
			letter.setPosition((PPB - (maxx - minx + 1)) / 2.0 - minx, (PPB - (maxy - miny + 1)) / 2.0 - miny);

			float diam = PPB / 4.0;
			float rad = PPB / 8.0;
			// draw final tile
			tile_texture[ch - 'A'].clear(sf::Color(0, 0, 0, 0));

			sf::RectangleShape rect;
			rect.setFillColor(sf::Color(255, 255, 175));

			rect.setSize(sf::Vector2f(PPB - diam, PPB));
			rect.setPosition(rad, 0);
			tile_texture[ch - 'A'].draw(rect);
			rect.setSize(sf::Vector2f(PPB, PPB - diam));
			rect.setPosition(0, rad);
			tile_texture[ch - 'A'].draw(rect);

			sf::CircleShape circle(rad);
			circle.setFillColor(sf::Color(255, 255, 175));

			circle.setPosition(0, 0);
			tile_texture[ch - 'A'].draw(circle);
			circle.setPosition(PPB - diam, 0);
			tile_texture[ch - 'A'].draw(circle);
			circle.setPosition(0, PPB - diam);
			tile_texture[ch - 'A'].draw(circle);
			circle.setPosition(PPB - diam, PPB - diam);
			tile_texture[ch - 'A'].draw(circle);

			tile_texture[ch - 'A'].draw(letter);
			tile_texture[ch - 'A'].display();

			// create tiles
			for (unsigned int i = 0; i < letter_count[ch - 'A']; i++)
				tiles[ch - 'A'].push_back(new Tile(ch));

			cerr << ch;
		}

		float time = clock.getElapsedTime().asSeconds();
		cerr << " (" << time << ")\n";
	}

	sf::Color background(22, 22, 22);

	sf::RenderWindow window(sf::VideoMode(1920, 1080), "Bananagrams", sf::Style::Titlebar);
	sf::View view = window.getDefaultView();
	view.setCenter(PPB / 2.0, PPB / 2.0);
	window.setView(view);

	int pos[2] = {0, 0};
	int delta[2] = {0, 0};
	float held[2] = {0, 0};

	float repeat_delay = 0.5;
	float repeat_speed = 0.1;

	float cursor_thickness = PPB / 16.0;
	sf::RectangleShape cursor(sf::Vector2f(PPB - cursor_thickness * 2, PPB - cursor_thickness * 2));
	cursor.setFillColor(sf::Color(0, 0, 0, 0));
	cursor.setOutlineThickness(cursor_thickness);
	cursor.setOutlineColor(sf::Color(0, 200, 0));

	std::vector<InputReader*> input_readers;

	Game game(&window);
	input_readers.push_back(&game);

	char ch = 'A' - 1;
	bool zoom_key = false;
	bool sprint_key = false;
	bool backspace = false;
	int next[2];
	int last[2] = {0, 0};
	VimControls controls(delta, &ch, &zoom_key, &backspace);
	input_readers.push_back(&controls);

	clock.restart();
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

		if (backspace)
		{
			backspace = false;

			// if the cursor is ahead of the last added character
			if (pos[0] == last[0] + next[0] && pos[1] == last[1] + next[1])
			{
				pos[0] = last[0];
				pos[1] = last[1];
				last[0] -= next[0];
				last[1] -= next[1];
			}

			auto tile = grid.remove(pos[0], pos[1]);
			if (tile != nullptr)
				tiles[tile->ch() - 'A'].push_back(tile);
		}
		else if (ch >= 'A' && ch <= 'Z')
		{
			if (tiles[ch - 'A'].size() > 0)
			{
				Tile* tile = grid.swap(pos[0], pos[1], tiles[ch - 'A'].back());
				tiles[ch - 'A'].pop_back();
				if (tile != nullptr)
					tiles[tile->ch() - 'A'].push_back(tile);
				next[0] = 0;
				next[1] = 0;
				if (pos[0] == last[0] + 1 && pos[1] == last[1])
					next[0] = 1;
				if (pos[0] == last[0] && pos[1] == last[1] + 1)
					next[1] = 1;
				last[0] = pos[0];
				last[1] = pos[1];
				pos[0] += next[0];
				pos[1] += next[1];
			}

			ch = 'A' - 1;
		}

		float time = clock.getElapsedTime().asSeconds();
		clock.restart();

		if (zoom_key)
		{
			view.zoom(1 + delta[1] * (sprint_key ? 2 : 1) * time);
		}
		else
		{
			for (unsigned int i = 0; i < 2; i++)
			{
				if (delta[i] == 0)
					held[i] = 0;
				else
				{
					if (held[i] == 0)
						pos[i] += delta[i] * (sprint_key ? 2 : 1);
					else
						while (held[i] > repeat_delay)
						{
							pos[i] += delta[i] * (sprint_key ? 2 : 1);
							held[i] -= repeat_speed;
						}
					held[i] += time;
				}
			}
		}

		auto center = view.getCenter();
		float max = time * 500.0 * (sprint_key ? 2 : 1);
		sf::Vector2f diff(pos[0] * PPB + PPB / 2.0 - center.x, pos[1] * PPB + PPB / 2.0 - center.y);
		float length = std::sqrt(diff.x * diff.x + diff.y * diff.y);
		if (length > max)
		{
			diff.x = (diff.x * max) / length;
			diff.y = (diff.y * max) / length;
		}
		view.move(diff.x, diff.y);
		// don't allow zooming past default
		auto size = view.getSize();
		// TODO bleh hardcoded size
		if (size.x < 1024 || size.y < 600)
			view.setSize(1024, 600);
		window.setView(view);

		cursor.setPosition(pos[0] * PPB + cursor_thickness, pos[1] * PPB + cursor_thickness);

		window.clear(background);
		grid.draw_on(window);
		window.draw(cursor);

		window.display();
	}

	// delete unused tiles
	for (char ch = 'A'; ch <= 'Z'; ch++)
		for (auto tile: tiles[ch - 'A'])
			delete tile;

	return 0;
}

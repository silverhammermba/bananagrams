#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <SFML/Graphics.hpp>

using std::cerr;
using std::endl;

#define PPB 64
#define DIAM PPB / 4.0
#define RAD PPB / 8.0
#define THICK PPB / 16.0
#define DTHICK PPB / 8.0

sf::RenderTexture tile_texture[26];

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
	font.loadFromFile("/usr/share/fonts/TTF/VeraMono.ttf");
	Grid grid;

	// create textures and tiles
	cerr << "Generating textures...\n";
	std::vector<Tile*> tiles[26];
	for (char ch = 'A'; ch <= 'Z'; ch++)
		tiles[ch - 'A'] = std::vector<Tile*>();
	{
		unsigned int miny = PPB;
		unsigned int maxy = 0;
		bool done_y = false;
		for (char ch = 'A'; ch <= 'Z'; ch++)
		{
			// TODO error check
			// TODO memory leak here?
			tile_texture[ch - 'A'].create(PPB, PPB);
			tile_texture[ch - 'A'].clear(sf::Color::Red);

			std::stringstream string;
			string << ch;
			sf::Text letter(string.str(), font, (PPB * 2) / 3.0);
			letter.setColor(sf::Color::Black);
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

			tile_texture[ch - 'A'].clear(sf::Color(0, 0, 0, 0));

			sf::RectangleShape rect;
			rect.setFillColor(sf::Color(255, 255, 175));

			rect.setSize(sf::Vector2f(PPB - DIAM, PPB));
			rect.setPosition(RAD, 0);
			tile_texture[ch - 'A'].draw(rect);
			rect.setSize(sf::Vector2f(PPB, PPB - DIAM));
			rect.setPosition(0, RAD);
			tile_texture[ch - 'A'].draw(rect);

			sf::CircleShape circle(RAD);
			circle.setFillColor(sf::Color(255, 255, 175));

			circle.setPosition(0, 0);
			tile_texture[ch - 'A'].draw(circle);
			circle.setPosition(PPB - DIAM, 0);
			tile_texture[ch - 'A'].draw(circle);
			circle.setPosition(0, PPB - DIAM);
			tile_texture[ch - 'A'].draw(circle);
			circle.setPosition(PPB - DIAM, PPB - DIAM);
			tile_texture[ch - 'A'].draw(circle);

			tile_texture[ch - 'A'].draw(letter);
			tile_texture[ch - 'A'].display();

			tiles[ch - 'A'].push_back(new Tile(ch));
		}
	}

	sf::Color background(22, 22, 22);

	sf::RenderWindow window(sf::VideoMode(1024, 600), "Bananagrams", sf::Style::Titlebar);

	int pos[2] = {0, 0};
	int delta[2] = {0, 0};
	float held[2] = {0, 0};

	float repeat_delay = 0.5;
	float repeat_speed = 0.1;

	sf::RectangleShape cursor(sf::Vector2f(PPB - DTHICK, PPB - DTHICK));
	cursor.setFillColor(sf::Color(0, 0, 0, 0));
	cursor.setOutlineThickness(THICK);
	cursor.setOutlineColor(sf::Color(0, 200, 0));

	sf::Clock clock;
	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed || (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape))
				window.close();
			else if (event.type == sf::Event::KeyPressed)
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
					default:
						break;
				}
				if (sf::Keyboard::A <= event.key.code && event.key.code <= sf::Keyboard::Z)
				{
					if (tiles[event.key.code - sf::Keyboard::A].size() > 0)
					{
						Tile* tile = grid.swap(pos[0], pos[1], tiles[event.key.code - sf::Keyboard::A].back());
						tiles[event.key.code - sf::Keyboard::A].pop_back();
						if (tile != nullptr)
							tiles[tile->ch() - 'A'].push_back(tile);
					}
				}
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
					default:
						break;
				}
			}
		}

		float time = clock.getElapsedTime().asSeconds();
		clock.restart();

		for (unsigned int i = 0; i < 2; i++)
		{
			if (delta[i] == 0)
				held[i] = 0;
			else
			{
				if (held[i] == 0)
					pos[i] += delta[i];
				else
					while (held[i] > repeat_delay)
					{
						pos[i] += delta[i];
						held[i] -= repeat_speed;
					}
				held[i] += time;
			}
		}

		cursor.setPosition(pos[0] * PPB + THICK, pos[1] * PPB + THICK);

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

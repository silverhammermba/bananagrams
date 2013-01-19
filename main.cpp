#include <iostream>
#include <sstream>
#include <vector>
#include <SFML/Graphics.hpp>

using std::cerr;
using std::endl;

#define PPB 32

sf::Font font;

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

	void set_pos(int x, int y)
	{
		sprite.setPosition(x * PPB, y * PPB);
	}

	void draw_on(sf::RenderWindow & window) const
	{
		window.draw(sprite);
	}
};

class InfiniteGrid
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
	InfiniteGrid() : grid()
	{
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
		// TODO raise error if tile is null
		unsigned int n = convert(x, y);
		if (n >= grid.size())
			// TODO catch failure
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
	font.loadFromFile("/usr/share/fonts/TTF/VeraMono.ttf");
	InfiniteGrid grid;

	cerr << "Generating textures...\n";
	// create tiles
	std::vector<Tile*> tiles;
	for (char ch = 'A'; ch <= 'Z'; ch++)
	{
		// TODO error check
		tile_texture[ch - 'A'].create(PPB, PPB);
		tile_texture[ch - 'A'].clear(sf::Color(255, 255, 175));

		std::stringstream string;
		string << ch;
		sf::Text letter(string.str(), font, 24);
		letter.setColor(sf::Color::Black);
		tile_texture[ch - 'A'].draw(letter);
		tile_texture[ch - 'A'].display();

		tiles.push_back(new Tile(ch));
	}

	sf::Color background(22, 22, 22);

	sf::RenderWindow window(sf::VideoMode(1024, 600), "Bananagrams", sf::Style::Titlebar);

	int pos[2] = {0, 0};
	int delta[2] = {0, 0};
	float held[2] = {0, 0};

	float repeat_delay = 0.5;
	float repeat_speed = 0.1;

	sf::RectangleShape cursor(sf::Vector2f(PPB, PPB));
	cursor.setFillColor(sf::Color(0, 0, 0, 0));
	cursor.setOutlineThickness(3);
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
					grid.swap(pos[0], pos[1], tiles[event.key.code - sf::Keyboard::A]);
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

		cursor.setPosition(pos[0] * PPB, pos[1] * PPB);

		window.clear(background);
		grid.draw_on(window);
		window.draw(cursor);

		window.display();
	}

	return 0;
}

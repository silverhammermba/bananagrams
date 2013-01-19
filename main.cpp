#include <sstream>
#include <dequeue>
#include <SFML/Graphics.hpp>

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
	int min[2];
	int max[2];
	std::dequeue<std::dequeue<Tile*>> grid;
public:
	InfiniteGrid() : min {0, 0}, max {-1, -1}, grid()
	{
	}

	Tile* get(int x, int y)
	{
		if (grid.size() == 0)
			return nullptr;
		if (min[0] <= x && x <= max[0] && min[1] <= y && y <= max[1])
			return grid[x][y];
		else
			return nullptr;
	}

	void set(int x, int y, Tile* tile)
	{
		if (grid.size() == 0)
		{
			grid.push_back(std::dequeue());
			grid[0].push_back(tile);
			min[0] = 0;
			min[1] = 0;
			max[0] = 0;
			max[1] = 0;
		}
		else
		{
			if (x < min[0])
			{
				for (int i = x; i < min[0]; i++)
					grid.push_front(std::dequeue(height));
				min[0] = x;
			}
			else if (x > max[0])
			{
				for (int i = x; i < max[0]; i++)
					grid.push_back(std::dequeue(height));
				max[0] = x;
			}
			if (y < min[1])
			{
				for (auto col : grid)
					for (int i = y; i < min[1]; i++)
						col->push_front(nullptr);
				min[1] = y;
			}
			else if (y > max[1])
			{
				for (auto col : grid)
					for (int i = y; i < min[1]; i++)
						col->push_back(nullptr);
				min[1] = y;
			}
		}
	}
};

int main()
{
	font.loadFromFile("/usr/share/fonts/TTF/VeraMono.ttf");

	// create tile textures
	for (char ch = 'A'; ch <= 'Z'; ch++)
	{
		sf::RectangleShape tile(sf::Vector2f(PPB, PPB));
		tile.setFillColor(sf::Color(255, 255, 175));
		tile_texture[ch - 'A'].draw(tile);

		std::stringstream string;
		string << ch;
		sf::Text letter(string.str(), font, 24);
		tile_texture[ch - 'A'].draw(letter);
		tile_texture[ch - 'A'].display();
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
	cursor.setOutlineThickness(5);
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
		window.draw(cursor);

		window.display();
	}

	return 0;
}

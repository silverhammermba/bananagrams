#include <SFML/Graphics.hpp>

#define PPB 32

sf::Font font;

int main()
{
	// set up text
	font.loadFromFile("/usr/share/fonts/TTF/VeraMono.ttf");

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

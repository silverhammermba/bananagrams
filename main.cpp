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
	sf::RectangleShape cursor(sf::Vector2f(PPB, PPB));
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
			else if (event.type == sf::Event::KeyPressed)
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

		pos[0] += delta[0];
		pos[1] += delta[1];

		cursor.setPosition(pos[0] * PPB, pos[1] * PPB);

		window.clear(background);
		window.draw(cursor);

		window.display();
	}

	return 0;
}

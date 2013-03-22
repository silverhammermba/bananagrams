#include <cctype>
#include <cmath>
#include <cstdlib>
#include <array>
#include <iostream>
#include <fstream>
#include <list>
#include <map>
#include <queue>
#include <string>
#include <sstream>
#include <vector>

#include <yaml-cpp/yaml.h>
#include <SFML/Graphics.hpp>

static const unsigned int PPB = 48;
static const sf::Vector2i X(1, 0);
static const sf::Vector2i Y(0, 1);
static const sf::Vector2i XY(1, 1);

extern sf::Font font;
extern sf::RenderTexture tile_texture[26];
extern std::map<std::string, std::string> dictionary;

// TODO better place for this?
// for classes that handle sf::Events
class InputReader
{
protected:
	bool finished;
public:
	InputReader() { finished = false; }
	virtual ~InputReader() {}

	inline bool is_finished() const
	{
		return finished;
	}

	virtual bool process_event(sf::Event& event)
	{
		return true;
	}
};

#include "menu.hpp"
#include "control.hpp"
#include "message.hpp"
#include "cursor.hpp"
#include "tile.hpp"
#include "grid.hpp"
#include "hand.hpp"
#include "buffer.hpp"

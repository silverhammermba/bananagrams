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

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <yaml-cpp/yaml.h>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>

static const unsigned int PPB {48};
static const sf::Vector2i X {1, 0};
static const sf::Vector2i Y {0, 1};
static const sf::Vector2i XY {1, 1};
static const sf::Vector2i ZERO {0, 0};

extern sf::Font font;
extern sf::RenderTexture tile_texture[26];
extern sf::View gui_view;

// TODO better place for this?
// for classes that handle sf::Events
class InputReader
{
protected:
	bool finished {false};
public:
	virtual ~InputReader() {}

	inline bool is_finished() const
	{
		return finished;
	}

	virtual bool process_event(sf::Event& event)
	{
		// intentionally unused parameter
		return true;
	}
};

#include "../common.hpp"
#include "control.hpp"
#include "message.hpp"
#include "cursor.hpp"
#include "tile.hpp"
#include "grid.hpp"
#include "hand.hpp"
#include "buffer.hpp"
#include "game.hpp"
#include "menu.hpp"

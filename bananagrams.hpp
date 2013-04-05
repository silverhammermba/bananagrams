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

static const unsigned int PPB {48};
static const sf::Vector2i X {1, 0};
static const sf::Vector2i Y {0, 1};
static const sf::Vector2i XY {1, 1};
static const sf::Vector2i ZERO {0, 0};

extern sf::Font font;
extern sf::RenderTexture tile_texture[26];
extern sf::View gui_view;

static const unsigned int letter_count[26]
{
	13, 3, 3, 6, 18, 3, 4, 3, 12, 2, 2, 5, 3, 8, 11, 3, 2, 9, 6, 9, 6, 3, 3, 2, 3, 2
//   A  B  C  D   E  F  G  H   I  J  K  L  M  N   O  P  Q  R  S  T  U  V  W  X  Y  Z
};

// insert x into list l at a random position
template <class T>
void random_insert(std::list<T>& l, T x)
{
	auto it = l.begin();
	auto pos = std::rand() % (l.size() + 1);
	for (unsigned int i = 0; i != pos && it != l.end(); it++, i++);
	l.insert(it, x);
}

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

#pragma GCC diagnostic ignored "-Wunused-parameter"
	virtual bool process_event(sf::Event& event)
	{
		return true;
	}
#pragma GCC diagnostic pop
};

#include "control.hpp"
#include "message.hpp"
#include "cursor.hpp"
#include "tile.hpp"
#include "grid.hpp"
#include "hand.hpp"
#include "buffer.hpp"
#include "game.hpp"
#include "menu.hpp"

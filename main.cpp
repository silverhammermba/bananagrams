#include <array>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <fstream>
#include <list>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <SFML/Graphics.hpp>

// embedded resources
#include "icon.hpp"

using std::array;
using std::cerr;
using std::endl;
using std::list;
using std::map;
using std::string;
using std::stringstream;
using std::vector;

static const int PPB = 48;
static const sf::Vector2i X(1, 0);
static const sf::Vector2i Y(0, 1);
static const sf::Vector2i XY(1, 1);

sf::RenderTexture tile_texture[26];
map<string, string> dictionary;

// structs for passing around state
struct State
{
	// general
	sf::RenderWindow* window;
	sf::View* gui_view;
	sf::View* grid_view;
	float zoom; // zoom factor for grid view
	bool switch_controls; // signal to switch control schemes
	bool transpose; // flip selection

	// keyboard
	sf::Vector2i delta; // cursor movement signal
	char ch; // tile to place
	bool ctrl; // if Ctrl is being held
	bool sprint; // if cursor movement should be fast
	bool kremove; // signal to remove a tile
	bool peel;
	bool dump;
	bool cut;
	bool paste;

	// mouse
	int pos[2]; // last position
	bool update; // signal to update cursor position
	bool mremove; // signal to remove tiles
	int wheel_delta; // amount to zoom
	bool start_selection;
	bool end_selection;
};

// ABC for classes that handle sf::Events
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

	virtual bool process_event(const sf::Event& event) = 0;
};

// class for handling game-related events
class Game : public InputReader
{
	State* state;
public:
	Game(State* s)
	{
		state = s;
	}

	virtual bool process_event(const sf::Event& event)
	{
		if (event.type == sf::Event::Closed || (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape))
		{
			state->window->close();

			finished = true;
			return false;
		}
		else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::F5)
			state->switch_controls = true;
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

class Tile
{
	char character;
	sf::Sprite sprite;
public:
	bool marked;

	Tile(char ch) :
		sprite(tile_texture[ch - 'A'].getTexture())
	{
		character = ch;
	}

	inline char ch() const
	{
		return character;
	}

	inline void set_pos(float x, float y)
	{
		sprite.setPosition(x, y);
	}

	inline void set_grid_pos(int x, int y)
	{
		sprite.setPosition(x * PPB, y * PPB);
	}

	inline void set_grid_pos(const sf::Vector2i& pos)
	{
		sprite.setPosition(pos.x * PPB, pos.y * PPB);
	}

	inline const sf::Color& get_color() const
	{
		return sprite.getColor();
	}

	inline void set_color(const sf::Color& color)
	{
		sprite.setColor(color);
	}

	inline const sf::Vector2f& get_pos() const
	{
		return sprite.getPosition();
	}

	inline void draw_on(sf::RenderWindow & window) const
	{
		window.draw(sprite);
	}
};

// for hwords and vwords vectors
namespace std
{
	template<> struct less<sf::Vector2i>
	{
		bool operator() (const sf::Vector2i& lhs, const sf::Vector2i& rhs)
		{
			return lhs.x < rhs.x || (lhs.x == rhs.x && lhs.y < rhs.y);
		}
	};
}

class Grid
{
	// TODO keep track of center, add command for centering
	vector<Tile*> grid;
	vector<string> defined;
	map<sf::Vector2i, bool> hwords;
	map<sf::Vector2i, bool> vwords;

	// for converting between 2D coords and vector positions
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

	// for checking connectedness of grid
	void traverse(int x, int y)
	{
		Tile* tile = get(x, y);
		if (tile == nullptr || tile->marked)
			return;

		tile->marked = true;
		traverse(x - 1, y);
		traverse(x + 1, y);
		traverse(x, y - 1);
		traverse(x, y + 1);
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

	// return the tile at the coords
	Tile* get(int x, int y) const
	{
		unsigned int n = convert(x, y);
		if (n >= grid.size())
			return nullptr;
		return grid[n];
	}

	// remove the tile at the coords and return it
	Tile* remove(int x, int y)
	{
		unsigned int n = convert(x, y);
		// if in bounds
		if (n < grid.size())
		{
			Tile* tile = grid[n];

			// return if nothing was changed
			if (tile == nullptr)
				return nullptr;

			// shrink grid, if possible
			grid[n] = nullptr;
			for (n = grid.size(); n > 0; --n)
				if (grid[n - 1] != nullptr)
					break;

			grid.resize(n, nullptr);

			// check for created words
			if (get(x + 1, y) != nullptr && get(x + 2, y) != nullptr)
				hwords[sf::Vector2i(x + 1, y)] = true;

			if (get(x, y + 1) != nullptr && get(x, y + 2) != nullptr)
				vwords[sf::Vector2i(x, y + 1)] = true;

			// check for destroyed words
			hwords.erase(sf::Vector2i(x, y));
			if (get(x - 2, y) == nullptr)
				hwords.erase(sf::Vector2i(x - 1, y));

			vwords.erase(sf::Vector2i(x, y));
			if (get(x, y - 2) == nullptr)
				vwords.erase(sf::Vector2i(x, y - 1));

			return tile;
		}
		return nullptr;
	}

	// exchange the tile at the coords for the given one, and return it
	Tile* swap(int x, int y, Tile* tile)
	{
		if (tile == nullptr)
			throw std::runtime_error("attempt to place NULL tile");
		unsigned int n = convert(x, y);
		if (n >= grid.size())
			grid.resize(n + 1, nullptr);
		Tile* swp = grid[n];
		tile->set_grid_pos(x, y);
		grid[n] = tile;

		if (swp != nullptr)
			return swp;

		// check for created words
		if (get(x - 1, y) == nullptr)
		{
			if (get(x + 1, y) != nullptr)
				hwords[sf::Vector2i(x, y)] = true;
		}
		else if (get(x - 2, y) == nullptr)
			hwords[sf::Vector2i(x - 1, y)] = true;

		if (get(x, y - 1) == nullptr)
		{
			if (get(x, y + 1) != nullptr)
				vwords[sf::Vector2i(x, y)] = true;
		}
		else if (get(x, y - 2) == nullptr)
			vwords[sf::Vector2i(x, y - 1)] = true;

		// check for destroyed words
		if (get(x + 1, y) != nullptr)
			hwords.erase(sf::Vector2i(x + 1, y));

		if (get(x, y + 1) != nullptr)
			vwords.erase(sf::Vector2i(x, y + 1));

		return nullptr;
	}

	inline Tile* get(const sf::Vector2i& pos)
	{
		return get(pos.x, pos.y);
	}

	inline Tile* remove(const sf::Vector2i& pos)
	{
		return remove(pos.x, pos.y);
	}

	inline Tile* swap(const sf::Vector2i& pos, Tile* tile)
	{
		return swap(pos.x, pos.y, tile);
	}

	// animate tiles
	void step(float time)
	{
		for (auto tile: grid)
		{
			if (tile != nullptr)
			{
				auto color = tile->get_color();
				tile->set_color(color + sf::Color(time * 300, time * 300, time * 300));
			}
		}
	}

	// check for connectedness and valid words
	bool is_valid(vector<string>& messages)
	{
		// need at least one word to be valid
		if (hwords.size() == 0 && vwords.size() == 0)
			return false;

		auto start = hwords.begin();
		if (hwords.size() == 0)
			start = vwords.begin();

		// grid must be continuous
		for (auto tile: grid)
			if (tile != nullptr)
				tile->marked = false;

		// starting points guaranteed to be non-null
		traverse(start->first.x, start->first.y);

		bool valid = true;
		for (auto tile: grid)
			if (tile != nullptr && !tile->marked)
			{
				valid = false;
				tile->set_color(sf::Color(255, 50, 50));
			}

		if (!valid)
		{
			messages.push_back("Your tiles are not all connected.");
			return false;
		}

		stringstream temp;
		map<string, vector<array<int, 3>>> words;
		vector<string> defns;
		Tile* tile;

		// get words
		for (auto& pair: hwords)
		{
			temp.str("");
			for (unsigned int x = pair.first.x; (tile = get(x, pair.first.y)) != nullptr; x++)
				temp << tile->ch();
			if (!words.count(temp.str()))
					words[temp.str()] = vector<array<int, 3>>();
			words[temp.str()].push_back(array<int, 3>{{pair.first.x, pair.first.y, 0}});
		}
		for (auto& pair: vwords)
		{
			temp.str("");
			for (unsigned int y = pair.first.y; (tile = get(pair.first.x, y)) != nullptr; y++)
				temp << tile->ch();
			if (!words.count(temp.str()))
					words[temp.str()] = vector<array<int, 3>>();
			words[temp.str()].push_back(array<int, 3>{{pair.first.x, pair.first.y, 1}});
		}

		// TODO only define the most recently spelled words
		// check words
		for (auto& word : words)
		{
			auto it = dictionary.find(word.first);

			// if invalid
			if (it == dictionary.end())
			{
				// if this is first error, clear definitions
				if (valid)
					messages.clear();
				valid = false;
				messages.push_back(word.first + " is not a word.");
				int coord[2];
				// color incorrect tiles
				for (auto& pos: word.second)
					for (coord[0] = pos[0], coord[1] = pos[1]; (tile = get(coord[0], coord[1])) != nullptr; coord[pos[2]]++)
						tile->set_color(sf::Color(255, 50, 50));
			}
			// if valid and defined
			else if (valid && std::rand() % 100 == 0 && it->second.length() > 0)
			{
				// check if we have already displayed the definition
				bool defd = false;
				for (string& wd : defined)
					if (word.first == wd)
					{
						defd = true;
						break;
					}
				if (!defd)
					for (string& wd : defns)
						if (word.first == wd)
						{
							defd = true;
							break;
						}

				if (!defd)
				{
					defns.push_back(word.first);
					messages.push_back(word.first + ": " + it->second);
				}
			}
		}

		// if error-free, keep track of displayed definitions
		if (valid)
			for (string& wd : defns)
				defined.push_back(wd);

		return valid;
	}

	inline void draw_on(sf::RenderWindow& window) const
	{
		for (auto tile: grid)
			if (tile != nullptr)
				tile->draw_on(window);
	}
};

class MouseControls : public InputReader
{
	State* state;
public:
	MouseControls(State* m)
	{
		state = m;
	}

	virtual bool process_event(const sf::Event& event)
	{
		switch(event.type)
		{
			case sf::Event::MouseButtonPressed:
				if (event.mouseButton.button == sf::Mouse::Left)
					state->start_selection = true;
				else if (event.mouseButton.button == sf::Mouse::Right)
					state->mremove = true;
				break;
			case sf::Event::MouseButtonReleased:
				state->update = true;
				if (event.mouseButton.button == sf::Mouse::Left)
					state->end_selection = true;
				else if (event.mouseButton.button == sf::Mouse::Right)
					state->mremove = false;
				break;
			case sf::Event::MouseMoved:
				{
					state->update = true;
					state->pos[0] = event.mouseMove.x;
					state->pos[1] = event.mouseMove.y;
				}
				break;
			case sf::Event::MouseWheelMoved:
				state->wheel_delta = event.mouseWheel.delta;
				break;
			default:
				break;
		}

		return true;
	}
};

class SimpleControls : public InputReader
{
	State* state;
public:
	SimpleControls(State* s)
	{
		state = s;
	}

	virtual bool process_event(const sf::Event& event)
	{
		if (event.type == sf::Event::KeyPressed)
		{
			switch (event.key.code)
			{
				case sf::Keyboard::Left:
					state->delta.x = -1;
					break;
				case sf::Keyboard::Right:
					state->delta.x = 1;
					break;
				case sf::Keyboard::Up:
					state->delta.y = -1;
					break;
				case sf::Keyboard::Down:
					state->delta.y = 1;
					break;
				case sf::Keyboard::LControl:
				case sf::Keyboard::RControl:
					state->ctrl = true;
					break;
				case sf::Keyboard::LShift:
				case sf::Keyboard::RShift:
					state->sprint = true;
					break;
				case sf::Keyboard::BackSpace:
					state->kremove = true;
					break;
				case sf::Keyboard::Space:
					state->peel = true;
					break;
				case sf::Keyboard::D:
					if (state->ctrl)
					{
						state->dump = true;
						return true;
					}
					break;
				case sf::Keyboard::F:
					if (state->ctrl)
					{
						state->transpose = true;
						return true;
					}
					break;
				case sf::Keyboard::X:
					if (state->ctrl)
					{
						state->cut = true;
						return true;
					}
					break;
				case sf::Keyboard::V:
					if (state->ctrl)
					{
						state->paste = true;
						return true;
					}
					break;
				default:
					break;
			}
			if (sf::Keyboard::A <= event.key.code && event.key.code <= sf::Keyboard::Z)
				state->ch = event.key.code - sf::Keyboard::A + 'A';
		}
		else if (event.type == sf::Event::KeyReleased)
		{
			switch (event.key.code)
			{
				case sf::Keyboard::Left:
				case sf::Keyboard::Right:
					state->delta.x = 0;
					break;
				case sf::Keyboard::Up:
				case sf::Keyboard::Down:
					state->delta.y = 0;
					break;
				case sf::Keyboard::LControl:
				case sf::Keyboard::RControl:
					state->ctrl = false;
					break;
				case sf::Keyboard::LShift:
				case sf::Keyboard::RShift:
					state->sprint = false;
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
	State* state;
public:
	VimControls(State* s)
	{
		state = s;
	}

	virtual bool process_event(const sf::Event& event)
	{
		if (event.type == sf::Event::KeyPressed)
		{
			// return to skip processing letter key as insert
			switch (event.key.code)
			{
				// directions
				case sf::Keyboard::Y:
					if (state->ctrl)
					{
						state->cut = true;
						return true;
					}
					if (shift)
						state->sprint = true;
					else
						break;
				case sf::Keyboard::H:
					if (!shift)
						break;
				case sf::Keyboard::Left:
					state->delta.x = -1;
					return true;
				case sf::Keyboard::O:
					if (shift)
						state->sprint = true;
					else
						break;
				case sf::Keyboard::L:
					if (!shift)
						break;
				case sf::Keyboard::Right:
					state->delta.x = 1;
					return true;
				case sf::Keyboard::I:
					if (shift)
						state->sprint = true;
					else
						break;
				case sf::Keyboard::K:
					if (!(shift || state->ctrl))
						break;
				case sf::Keyboard::Up:
					state->delta.y = -1;
					return true;
				case sf::Keyboard::U:
					if (shift)
						state->sprint = true;
					else
						break;
				case sf::Keyboard::J:
					if (!(shift || state->ctrl))
						break;
				case sf::Keyboard::Down:
					state->delta.y = 1;
					return true;
				// special functions
				case sf::Keyboard::F:
					if (shift)
					{
						state->transpose = true;
						return true;
					}
				case sf::Keyboard::P:
					if (shift || state->ctrl)
					{
						state->paste = true;
						return true;
					}
					break;
				// modifier keys
				case sf::Keyboard::LControl:
				case sf::Keyboard::RControl:
					state->ctrl = true;
					break;
				case sf::Keyboard::LShift:
				case sf::Keyboard::RShift:
					shift = true;
					break;
				case sf::Keyboard::X:
					if (!shift)
						break;
				case sf::Keyboard::BackSpace:
					state->kremove = true;
					return true;
				case sf::Keyboard::D:
					if (!shift)
						break;
					state->dump = true;
					return true;
				case sf::Keyboard::Space:
					state->peel = true;
					break;
				default:
					break;
			}
			if (sf::Keyboard::A <= event.key.code && event.key.code <= sf::Keyboard::Z)
				state->ch = event.key.code - sf::Keyboard::A + 'A';
		}
		else if (event.type == sf::Event::KeyReleased)
		{
			switch (event.key.code)
			{
				//YUIO
				//HJKL
				case sf::Keyboard::Y:
				case sf::Keyboard::O:
					state->sprint = false;
				case sf::Keyboard::H:
				case sf::Keyboard::L:
				case sf::Keyboard::Left:
				case sf::Keyboard::Right:
					state->delta.x = 0;
					break;
				case sf::Keyboard::U:
				case sf::Keyboard::I:
					state->sprint = false;
				case sf::Keyboard::J:
				case sf::Keyboard::K:
				case sf::Keyboard::Up:
				case sf::Keyboard::Down:
					state->delta.y = 0;
					break;
				case sf::Keyboard::LControl:
				case sf::Keyboard::RControl:
					state->ctrl = false;
					if (!shift)
						state->delta.y = 0;
					break;
				case sf::Keyboard::LShift:
				case sf::Keyboard::RShift:
					shift = false;
					state->sprint = false;
					state->delta.x = 0;
					state->delta.y = 0;
					break;
				default:
					break;
			}
		}
		return true;
	}
};

// TODO inefficient
class Hand : public InputReader
{
	vector<Tile*> tiles[26];
	sf::View* gui_view;
	list<Tile*> scram; // for shuffle
	list<Tile*> sort; // for ordered
	list<Tile*> single; // for counts
	sf::Text number[26];

	// position tiles in list in nice rows
	void position_list(list<Tile*>& l)
	{
		if (l.size() == 0)
			return;
		auto size = gui_view->getSize();
		float padding = PPB / 8.f;
		float min_width = PPB + padding;
		// leave PPB/2 space on either side of tiles, one tile gets full PPB width
		unsigned int max_per_row = ((int)size.x - PPB - padding * 2) / (int)min_width + 1;
		unsigned int rows = l.size() / max_per_row + (l.size() > (l.size() / max_per_row) * max_per_row ? 1 : 0);
		auto tile = l.begin();
		for (unsigned int i = 0; tile != l.end(); i++, tile++)
		{
			// number of tiles in this row
			unsigned int row_size = i >= (l.size() / max_per_row) * max_per_row ? l.size() % max_per_row : max_per_row;
			float room_per_tile = row_size == 1 ? 0 : (size.x - PPB - padding * 2) / (row_size - 1);
			// maximum PPB/4 spacing between tiles
			if (room_per_tile > (PPB * 5) / 4.0)
				room_per_tile = (PPB * 5) / 4.0;
			// this should be <= size.x - PPB
			float room_required = room_per_tile * (row_size - 1) + PPB;
			(*tile)->set_pos((i % max_per_row) * room_per_tile + (size.x - room_required) / 2.f, size.y - (rows - (i / max_per_row)) * (PPB + padding));
		}
	}

	void counts(sf::RenderWindow& window)
	{
		position_list(single);

		for (auto tile: single)
		{
			tile->draw_on(window);
			number[tile->ch() - 'A'].setPosition(tile->get_pos() + sf::Vector2f(PPB / 32.0, 0));
			window.draw(number[tile->ch() - 'A']);
		}
	}

	void stacks(sf::RenderWindow& window)
	{
		auto size = gui_view->getSize();
		unsigned int nonempty = 0;
		for (char ch = 'A'; ch <= 'Z'; ch++)
			if (has_any(ch))
				nonempty++;
		if (nonempty == 0)
			return;
		float padding = PPB / 8.f;
		float room_per_tile = nonempty == 1 ? 0 : (size.x - PPB - padding * 2) / (float)(nonempty - 1);
		if (room_per_tile > (PPB * 5) / 3.0)
			room_per_tile = (PPB * 5) / 3.0;
		// center tiles
		float x = (size.x - room_per_tile * (nonempty - 1) - PPB) / 2.0;
		for (char ch = 'A'; ch <= 'Z'; ch++)
		{
			if (has_any(ch))
			{
				unsigned int i = 0;
				for (auto tile: tiles[ch - 'A'])
				{
					tile->set_pos(x + (i * PPB) / 16.f, size.y - PPB - padding - (i * PPB) / 16.f);
					i++;
				}
				x += room_per_tile;
			}
		}

		for (char ch = 'Z'; ch >= 'A'; --ch)
			for (auto tile: tiles[ch - 'A'])
				tile->draw_on(window);
	}

	void ordered(sf::RenderWindow& window)
	{
		position_list(sort);

		for (auto tile: sort)
			tile->draw_on(window);
	}

	void scrambled(sf::RenderWindow& window)
	{
		position_list(scram);

		for (auto tile: scram)
			tile->draw_on(window);
	}

	void (Hand::*draw_func)(sf::RenderWindow&) = &Hand::scrambled;

	void reshuffle()
	{
		scram.clear();
		for (char ch = 'A'; ch <= 'Z'; ch++)
		{
			for (auto tile: tiles[ch - 'A'])
			{
				auto it = scram.begin();
				auto j = std::rand() % (scram.size() + 1);
				for (unsigned int i = 0; i != j && it != scram.end(); it++, i++);
				scram.insert(it, tile);
			}
		}
	}

public:
	Hand(sf::View* v, const sf::Font& font)
	{
		gui_view = v;

		// prepare counts numbers
		for (char ch = 'A'; ch <= 'Z'; ch++)
		{
			number[ch - 'A'].setFont(font);
			number[ch - 'A'].setCharacterSize(PPB / 4.0);
			number[ch - 'A'].setColor(sf::Color::Black);
		}
	}

	~Hand()
	{
		for (char ch = 'A'; ch <= 'Z'; ch++)
			for (auto tile : tiles[ch - 'A'])
				delete tile;
	}

	void add_tile(Tile* tile)
	{
		tiles[tile->ch() - 'A'].push_back(tile);

		tile->set_color(sf::Color::White);

		// update persistent structures
		scram.push_back(tile);
		if (tiles[tile->ch() - 'A'].size() == 1)
		{
			for (auto it = single.begin(); ; it++)
				if (it == single.end() || (*it)->ch() > tile->ch())
				{
					single.insert(it, tiles[tile->ch() - 'A'][0]);
					break;
				}
		}
		stringstream str;
		str << tiles[tile->ch() - 'A'].size();
		number[tile->ch() - 'A'].setString(str.str());
		for (auto it = sort.begin(); ; it++)
			if (it == sort.end() || (*it)->ch() >= tile->ch())
			{
				sort.insert(it, tile);
				break;
			}
	}

	inline bool has_any(char ch) const
	{
		return tiles[ch - 'A'].size() > 0;
	}

	Tile* remove_tile(char ch)
	{
		Tile* tile = nullptr;

		if (has_any(ch))
		{
			tile = tiles[ch - 'A'].back();
			tiles[ch - 'A'].pop_back();

			// update persistent structures
			scram.remove(tile);
			sort.remove(tile);
			if (has_any(tile->ch()))
			{
				stringstream str;
				str << tiles[tile->ch() - 'A'].size();
				number[tile->ch() - 'A'].setString(str.str());
			}
			else
				single.remove(tile);
		}

		return tile;
	}

	virtual bool process_event(const sf::Event& event)
	{
		if (event.type == sf::Event::KeyPressed)
		{
			switch (event.key.code)
			{
				case sf::Keyboard::F1:
					if (draw_func == &Hand::scrambled)
						reshuffle();
					else
						draw_func = &Hand::scrambled;
					break;
				case sf::Keyboard::F2:
					draw_func = &Hand::ordered;
					break;
				case sf::Keyboard::F3:
					draw_func = &Hand::counts;
					break;
				case sf::Keyboard::F4:
					draw_func = &Hand::stacks;
					break;
				default:
					break;
			}
		}
		return true;
	}

	inline void draw_on(sf::RenderWindow& window)
	{
		(this->*draw_func)(window);
	}
};

class CutBuffer
{
	sf::Vector2i pos;
	sf::Vector2i size;
	vector<Tile*> tiles;

	// for when CutBuffer is done being used
	void finish()
	{
		tiles.clear();
	}
public:
	CutBuffer(Grid& grid, int left, int top, int width, int height)
	{
		sf::Vector2i min(left + width - 1, top + height - 1);
		sf::Vector2i max(left, top);

		// try to shrink selection
		for (int i = left; i < left + width; i++)
			for (int j = top; j < top + height; j++)
			{
				auto tile = grid.get(i, j);
				if (tile != nullptr)
				{
					if (i < min.x)
						min.x = i;
					if (i > max.x)
						max.x = i;
					if (j < min.y)
						min.y = j;
					if (j > max.y)
						max.y = j;
				}
			}

		// if nonempty
		if (min.x <= max.x && min.y <= max.y)
		{
			size = max - min + XY;
			pos = (max + min) / 2;

			for (int i = min.x; i <= max.x; i++)
				for (int j = min.y; j <= max.y; j++)
				{
					Tile* tile = grid.remove(i, j);
					if (tile != nullptr)
						tile->set_color(sf::Color(255, 255, 255, 100));
					tiles.push_back(tile);
				}
		}
	}

	~CutBuffer()
	{
		for (auto tile : tiles)
			if (tile != nullptr)
				delete tile;
	}

	inline bool is_empty() const
	{
		return tiles.size() == 0;
	}

	void transpose()
	{
		vector<Tile*> temp(tiles.size(), nullptr);
		for (unsigned int i = 0; i < tiles.size(); i++)
			temp[(i % size.y) * size.x + i / size.y] = tiles[i];
		for (unsigned int i = 0; i < tiles.size(); i++)
			tiles[i] = temp[i];
		unsigned int tmp = size.x;
		size.x = size.y;
		size.y = tmp;

		set_pos(pos);
	}

	// put tiles back in grid, returning displaced tiles to hand
	void paste(Grid& grid, Hand& hand)
	{
		for (int i = 0; i < size.x; i++)
			for (int j = 0; j < size.y; j++)
			{
				auto tile = tiles[i * size.y + j];
				if (tile != nullptr)
				{
					tile->set_color(sf::Color::White);

					Tile* r = grid.swap(sf::Vector2i(i, j) + pos - size / 2, tile);
					if (r != nullptr)
						hand.add_tile(r);
				}
			}
		finish();
	}

	// return tiles to hand
	void clear(Hand& hand)
	{
		for (auto tile : tiles)
			if (tile != nullptr)
				hand.add_tile(tile);
		finish();
	}

	void set_pos(const sf::Vector2i& p)
	{
		pos = p;

		for (int i = 0; i < size.x; i++)
			for (int j = 0; j < size.y; j++)
			{
				auto tile = tiles[i * size.y + j];

				if (tile != nullptr)
					tile->set_grid_pos(sf::Vector2i(i, j) + pos - size / 2);
			}
	}

	inline void draw_on(sf::RenderWindow & window) const
	{
		for (auto tile: tiles)
			if (tile != nullptr)
				tile->draw_on(window);
	}
};

class Message
{
	sf::Text message;
	float lifetime = 0;
public:
	Message(const string& mes, const sf::Font& font, unsigned int size = 20, const sf::Color& color = sf::Color::Black) : message(mes, font, size)
	{
		message.setColor(color);
	}

	inline void age(float time)
	{
		lifetime += time;
	}

	inline float age() const
	{
		return lifetime;
	}

	inline void set_pos(float x, float y)
	{
		message.setPosition(x, y);
	}

	inline float get_height() const
	{
		return message.getGlobalBounds().height;
	}

	inline void draw_on(sf::RenderWindow& window) const
	{
		window.draw(message);
	}
};

class MessageQueue
{
	list<Message*> messages;
	sf::Font font;
	float bottom;
	static constexpr float padding = 10;
public:
	enum severity_t {LOW, HIGH};

	MessageQueue(const sf::Font& f) : font(f)
	{
		bottom = 0;
	}

	void add(const string& message, severity_t severity)
	{
		sf::Color color;
		unsigned int size;
		switch (severity)
		{
			case LOW:
				color = sf::Color::White;
				size = 12;
				break;
			case HIGH:
				color = sf::Color::Red;
				size = 18;
				break;
			default:
				color = sf::Color::Black;
				size = 12;
		}
		messages.push_back(new Message(message, font, size, color));
		messages.back()->set_pos(padding, bottom + padding);
		bottom += padding + messages.back()->get_height();
	}

	// TODO inefficient?
	void age(float time)
	{
		bool change = false;
		for (auto mess = messages.begin(); mess != messages.end();)
		{
			(*mess)->age(time);
			if ((*mess)->age() > 5)
			{
				mess = messages.erase(mess);
				change = true;
			}
			else
				mess++;
		}
		if (change)
		{
			bottom = 0;
			for (auto message : messages)
			{
				message->set_pos(padding, bottom + padding);
				bottom += padding + message->get_height();
			}
		}
	}

	inline void draw_on(sf::RenderWindow& window) const
	{
		for (auto message : messages)
			message->draw_on(window);
	}
};

class Cursor
{
	sf::Vector2i pos;
	float outline_thickness;
	sf::RectangleShape cursor;
public:
	Cursor(float thick, sf::Color fill, sf::Color outline)
		: pos(0, 0), cursor(sf::Vector2f(PPB - thick * 2, PPB - thick * 2))
	{
		cursor.setOutlineThickness(outline_thickness = thick);

		cursor.setFillColor(fill);
		cursor.setOutlineColor(outline);
	}

	inline const sf::Vector2i& get_pos() const
	{
		return pos;
	}

	inline sf::Vector2f get_center() const
	{
		return ((sf::Vector2f)pos + sf::Vector2f(0.5, 0.5)) * (float)PPB;
	}

	inline void move(const sf::Vector2i& d)
	{
		set_pos(pos + d);
	}

	void set_pos(const sf::Vector2i& p)
	{
		pos = p;

		cursor.setPosition((sf::Vector2f)(pos * PPB) + (sf::Vector2f)XY * cursor.getOutlineThickness());
	}

	inline void draw_on(sf::RenderWindow& window) const
	{
		window.draw(cursor);
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
	window.setKeyRepeatEnabled(false);

	sf::View gui_view = window.getDefaultView();
	sf::View grid_view = window.getDefaultView();
	grid_view.setCenter(PPB / 2.0, PPB / 2.0);

	vector<InputReader*> input_readers;

	State state;
	state.window = &window;
	state.gui_view = &gui_view;
	state.grid_view = &grid_view;
	state.zoom = 1;
	state.switch_controls = false;

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
	while (window.isOpen())
	{
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
			stringstream string;
			string << load_char;
			sf::Text letter(string.str(), font, (PPB * 2) / 3.0);
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
	MessageQueue messages(font);

	input_readers.push_back(&hand);

	Cursor cursor(PPB / 16.0, sf::Color(0, 0, 0, 0), sf::Color(0, 200, 0));
	Cursor mcursor(PPB / 16.0, sf::Color(0, 0, 0, 0), sf::Color(0, 200, 0, 80));

	sf::Vector2i last(-1, 0);
	sf::Vector2i next(0, 0);

	CutBuffer* buffer = nullptr;
	bool selected = false;
	bool selecting = false;
	// mouse press and release positions
	sf::Vector2i sel1;
	sf::Vector2i sel2;
	unsigned int sel_size[2];
	float selection_thickness = 1;
	sf::RectangleShape selection;
	selection.setFillColor(sf::Color(255, 255, 255, 25));
	selection.setOutlineThickness(selection_thickness);
	selection.setOutlineColor(sf::Color::White);

	// keyboard
	state.delta = {0, 0};
	state.ch = 'A' - 1;
	state.ctrl = false;
	state.sprint = false;
	state.kremove = false;
	state.peel = false;
	state.dump = false;
	state.cut = false;
	state.paste = false;

	// mouse
	state.pos[0] = 0;
	state.pos[1] = 0;
	state.update = false;
	state.mremove = false;
	state.wheel_delta = 0;
	state.start_selection = false;
	state.end_selection = false;

	state.transpose = false;

	MouseControls mouse(&state);
	input_readers.push_back(&mouse);

	SimpleControls scontrols(&state);
	VimControls vcontrols(&state);
	input_readers.push_back(&scontrols);

	// for controlling key repeat
	sf::Vector2f held(0, 0);
	float repeat_delay = 0.3;
	float repeat_speed = 0.07;

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

		if (state.switch_controls)
		{
			InputReader* controls = input_readers.back();
			input_readers.pop_back();
			if (controls == &scontrols)
			{
				input_readers.push_back(&vcontrols);
				messages.add("Switched to Vim controls.", MessageQueue::LOW);
			}
			else if (controls == &vcontrols)
			{
				input_readers.push_back(&scontrols);
				messages.add("Switched to simple controls.", MessageQueue::LOW);
			}
			else
			{
				cerr << "Failed to switch controls!\n";
				return 1;
			}
			state.switch_controls = false;
		}

		// mouse moved
		if (state.update)
		{
			// TODO need to do this when window is resized
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
			// normalize selection
			sel_size[0] = std::abs(sel1.x - sel2.x) + 1;
			sel_size[1] = std::abs(sel1.y - sel2.y) + 1;
			// update selection rect
			// TODO make selection rect a cursor
			selection.setSize(sf::Vector2f(PPB * sel_size[0] - selection_thickness * 2, PPB * sel_size[1] - selection_thickness * 2));
			selection.setPosition(std::min(sel1.x, sel2.x) * PPB + selection_thickness, std::min(sel1.y, sel2.y) * PPB + selection_thickness);
		}

		// if left click release
		if (state.end_selection)
		{
			selecting = false;
			selected = true;

			// if the selection was only 1 square, do something else
			if (sel_size[0] == 1 && sel_size[1] == 1)
			{
				selected = false;
				// move cursor to mouse cursor
				cursor.set_pos(mcursor.get_pos());

				// if ctrl, try to place last tile
				if (state.ctrl)
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
						messages.add("You do not have any tiles.", MessageQueue::LOW);
					else if (last > 'Z')
						messages.add("You have too many letters to place using the mouse.", MessageQueue::HIGH);
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

		if (state.cut)
		{
			if (buffer == nullptr)
			{
				if (selected)
				{
					buffer = new CutBuffer(grid, std::min(sel1.x, sel2.x), std::min(sel1.y, sel2.y), sel_size[0], sel_size[1]);

					if (buffer->is_empty())
					{
						messages.add("Nothing selected.", MessageQueue::LOW);
						delete buffer;
						buffer = nullptr;
					}
				}
				else
					messages.add("Nothing selected.", MessageQueue::LOW);

				selected = false;
			}
			else
			{
				buffer->clear(hand);
				delete buffer;
				buffer = nullptr;
				messages.add("Added cut tiles back to your hand.", MessageQueue::LOW);
			}

			state.cut = false;
		}

		if (state.transpose)
		{
			if (buffer != nullptr)
				buffer->transpose();

			state.transpose = false;
		}

		if (state.paste)
		{
			if (buffer != nullptr)
			{
				buffer->paste(grid, hand);
				delete buffer;
				buffer = nullptr;
			}
			else
				messages.add("Cannot paste: no tiles were cut.", MessageQueue::LOW);

			state.paste = false;
		}

		if (state.mremove)
		{
			// remove tile
			Tile* tile = grid.remove(mcursor.get_pos());
			if (tile != nullptr)
				hand.add_tile(tile);
		}

		if (state.dump)
		{
			if (bunch.size() >= 3)
			{
				auto dumped = grid.remove(cursor.get_pos());
				if (dumped == nullptr)
					messages.add("You need to select a tile to dump.", MessageQueue::LOW);
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
				messages.add("There are not enough tiles left to dump!", MessageQueue::HIGH);

			state.dump = false;
		}

		if (state.peel)
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
							messages.add(message, MessageQueue::LOW);
					}
					else
						messages.add("You win!", MessageQueue::LOW);
				}
				else
					for (auto message : mess)
						messages.add(message, MessageQueue::HIGH);
			}
			else
				messages.add("You have not used all of your letters.", MessageQueue::HIGH);
			state.peel = false;
		}

		// if backspace
		if (state.kremove)
		{
			state.kremove = false;

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
		if (state.ch >= 'A' && state.ch <= 'Z')
		{
			bool placed = false;

			// if space is empty or has a different letter
			if (grid.get(cursor.get_pos()) == nullptr || grid.get(cursor.get_pos())->ch() != state.ch)
			{
				if (hand.has_any(state.ch))
				{
					Tile* tile = grid.swap(cursor.get_pos(), hand.remove_tile(state.ch));

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
				letter << state.ch;
				messages.add("You are out of " + letter.str() + "s!", MessageQueue::HIGH);
			}

			// clear character to place
			state.ch = 'A' - 1;
		}

		// frame-time-dependent stuff
		float time = clock.getElapsedTime().asSeconds();
		clock.restart();

		messages.age(time);

		// TODO scale cursor outline thickness with zoom
		// zoom with mouse wheel
		if (state.wheel_delta < 0 || (state.wheel_delta > 0 && state.zoom > 1))
		{
			sf::Vector2f before((state.pos[0] * gsize.x) / wsize.x + center.x - gsize.x / 2, (state.pos[1] * gsize.y) / wsize.y + center.y - gsize.y / 2);
			grid_view.zoom(1 - state.wheel_delta * time * 2);
			gsize = grid_view.getSize();
			sf::Vector2f after((state.pos[0] * gsize.x) / wsize.x + center.x - gsize.x / 2, (state.pos[1] * gsize.y) / wsize.y + center.y - gsize.y / 2);
			grid_view.move(before - after);

			state.wheel_delta = 0;

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

		if (state.ctrl)
			grid_view.zoom(1 + state.delta.y * (state.sprint ? 2 : 1) * time);
		else
		{
			sf::Vector2i delta(0, 0);

			if (state.delta.x == 0)
				held.x = 0;
			else
			{
				if (held.x == 0)
					delta.x = state.delta.x;
				held.x += time;
			}
			if (state.delta.y == 0)
				held.y = 0;
			else
			{
				if (held.y == 0)
					delta.y = state.delta.y;
				held.y += time;
			}

			while (held.x > repeat_delay)
			{
				delta += X * state.delta.x;
				held.x -= repeat_speed;
			}

			while (held.y > repeat_delay)
			{
				delta += Y * state.delta.y;
				held.y -= repeat_speed;
			}

			cursor.move(delta);
		}

		// these might have changed due to zooming
		center = grid_view.getCenter();
		gsize = grid_view.getSize();
		sf::Vector2f spos = cursor.get_center();
		// TODO refactor
		// measure difference from a box in the center of the screen
		sf::Vector2f diff((std::abs(spos.x - center.x) > gsize.x / 4 ? spos.x - center.x - (spos.x >= center.x ? gsize.x / 4 : gsize.x / -4) : 0), (std::abs(spos.y - center.y) > gsize.y / 4 ? spos.y  - center.y - (spos.y >= center.y ? gsize.y / 4 : gsize.y / -4) : 0));
		// TODO is there a better movement function?
		grid_view.move(diff.x / (1.0 + time * 400), diff.y / (1.0 + time * 400));

		// don't allow zooming in past 1x
		if (gsize.x < wsize.x || gsize.y < wsize.y)
			grid_view.setSize(wsize.x, wsize.y);
		state.zoom = grid_view.getSize().x / wsize.x;

		// animate tiles
		grid.step(time);

		// draw
		window.clear(background);

		window.setView(grid_view);
		grid.draw_on(window);
		if (selecting || selected)
			window.draw(selection);
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

	return 0;
}

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

using std::cerr;
using std::endl;
using std::list;
using std::string;
using std::stringstream;
using std::vector;

#define PPB 64

sf::RenderTexture tile_texture[26];
std::map<string, string> dictionary;

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

	char ch() const
	{
		return character;
	}

	void set_pos(float x, float y)
	{
		sprite.setPosition(x, y);
	}

	const sf::Vector2f& get_pos() const
	{
		return sprite.getPosition();
	}

	void draw_on(sf::RenderWindow & window) const
	{
		window.draw(sprite);
	}
};

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
	vector<Tile*> grid;
	std::map<sf::Vector2i, bool> hwords;
	std::map<sf::Vector2i, bool> vwords;

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

	Tile* swap(int x, int y, Tile* tile)
	{
		if (tile == nullptr)
			throw std::runtime_error("attempt to place NULL tile");
		unsigned int n = convert(x, y);
		if (n >= grid.size())
			// TODO catch failure?
			grid.resize(n + 1, nullptr);
		Tile* swp = grid[n];
		tile->set_pos(x * PPB, y * PPB);
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

	bool is_valid(string* message)
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

		// guaranteed to be non-null
		traverse(start->first.x, start->first.y);

		bool valid = true;
		for (auto tile: grid)
			if (tile != nullptr && !tile->marked)
			{
				valid = false;
				break;
			}

		if (!valid)
		{
			*message = "Your tiles are not all connected.";
			return false;
		}

		stringstream word;
		Tile* tile;
		for (auto& pair: hwords)
		{
			word.str("");
			for (unsigned int x = pair.first.x; (tile = get(x, pair.first.y)) != nullptr; x++)
				word << (char)(tile->ch() - 'A' + 'a');

			auto it = dictionary.find(word.str());

			if (it == dictionary.end())
			{
				// TODO this seems real fucking dirty
				char* str = new char[word.str().length() + 1];
				std::strcpy(str, word.str().c_str());
				for (unsigned int i = 0; i < word.str().length(); i++)
					str[i] = str[i] - 'a' + 'A';
				*message = string(str) + " is not a valid word.";
				delete[] str;
				valid = false;
				break;
			}
		}

		if (!valid)
			return false;

		for (auto& pair: vwords)
		{
			word.str("");
			for (unsigned int y = pair.first.y; (tile = get(pair.first.x, y)) != nullptr; y++)
				word << (char)(tile->ch() - 'A' + 'a');

			auto it = dictionary.find(word.str());

			if (it == dictionary.end())
			{
				// TODO same here. not DRY
				char* str = new char[word.str().length() + 1];
				std::strcpy(str, word.str().c_str());
				for (unsigned int i = 0; i < word.str().length(); i++)
					str[i] = str[i] - 'a' + 'A';
				*message = string(str) + " is not a valid word.";
				delete[] str;
				valid = false;
				break;
			}
		}

		return valid;
	}

	void draw_on(sf::RenderWindow& window) const
	{
		for (auto tile: grid)
			if (tile != nullptr)
				tile->draw_on(window);
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
	Grid* grid;
	bool* peel;
public:
	VimControls(int* d, char* c, bool* z, bool* b, Grid* g, bool* p)
	{
		delta = d;
		ch = c;
		zoom_key = z;
		backspace = b;
		grid = g;
		peel = p;
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
				case sf::Keyboard::Space:
					*peel = true;
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

class TileDisplay : public InputReader
{
	bool reposition = true;
	vector<Tile*>* tiles;
	sf::RenderWindow* window;
	list<Tile*> scram; // for shuffle
	list<Tile*> sort; // for ordered
	list<Tile*> single; // for counts
	sf::Vector2u size;
	sf::Text number[26];

	// position tiles in list in nice rows
	void position_list(list<Tile*>& l)
	{
		if (l.size() == 0)
			return;
		float padding = PPB / 8.f;
		float min_width = PPB + padding;
		// leave PPB/2 space on either side of tiles, one tile gets full PPB width
		unsigned int max_per_row = (size.x - PPB - padding * 2) / (int)min_width + 1;
		unsigned int rows = l.size() / max_per_row + (l.size() > (l.size() / max_per_row) * max_per_row ? 1 : 0);
		auto tile = l.begin();
		for (unsigned int i = 0; tile != l.end(); i++, tile++)
		{
			// number of tiles in this row
			unsigned int row_size = i >= (l.size() / max_per_row) * max_per_row ? l.size() % max_per_row : max_per_row;
			float room_per_tile = row_size == 1 ? 0 : (size.x - PPB - padding * 2) / (float)(row_size - 1);
			// maximum PPB/4 spacing between tiles
			if (room_per_tile > (PPB * 5) / 4.0)
				room_per_tile = (PPB * 5) / 4.0;
			// this should be <= size.x - PPB
			float room_required = room_per_tile * (row_size - 1) + PPB;
			(*tile)->set_pos((i % max_per_row) * room_per_tile + (size.x - room_required) / 2.f, size.y - (rows - (i / max_per_row)) * (PPB + padding));
		}
	}

	void counts()
	{
		if (reposition)
		{
			position_list(single);
			reposition = false;
		}

		for (auto tile: single)
		{
			tile->draw_on(*window);
			number[tile->ch() - 'A'].setPosition(tile->get_pos() + sf::Vector2f(PPB / 32.0, 0));
			window->draw(number[tile->ch() - 'A']);
		}
	}

	void stacks()
	{
		if (reposition)
		{
			unsigned int nonempty = 0;
			for (char ch = 'A'; ch <= 'Z'; ch++)
				if (tiles[ch - 'A'].size() > 0)
					nonempty++;
			if (nonempty == 0)
				return;
			float padding = PPB / 8.f;
			float room_per_tile = nonempty == 1 ? 0 : (size.x - PPB - padding * 2) / (float)(nonempty - 1);
			float x = padding;
			for (char ch = 'A'; ch <= 'Z'; ch++)
			{
				if (tiles[ch - 'A'].size() > 0)
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
			reposition = false;
		}

		for (char ch = 'Z'; ch >= 'A'; --ch)
			for (auto tile: tiles[ch - 'A'])
				tile->draw_on(*window);
	}

	void ordered()
	{
		if (reposition)
		{
			position_list(sort);
			reposition = false;
		}
		for (auto tile: sort)
			tile->draw_on(*window);
	}

	void scrambled()
	{
		if (reposition)
		{
			position_list(scram);
			reposition = false;
		}
		for (auto tile: scram)
			tile->draw_on(*window);
	}

	void (TileDisplay::*draw_func)(void) = &TileDisplay::scrambled;

	void reshuffle()
	{
		reposition = true;
		scram.clear();
		for (char ch = 'A'; ch <= 'Z'; ch++)
		{
			for (auto tile: tiles[ch - 'A'])
			{
				auto it = scram.begin();
				auto pos = std::rand() % (scram.size() + 1);
				for (unsigned int i = 0; i != pos && it != scram.end(); it++, i++);
				scram.insert(it, tile);
			}
		}
	}

public:
	TileDisplay(sf::RenderWindow* win, vector<Tile*>* t, const sf::Font& font) : size(win->getSize())
	{
		window = win;
		tiles = t;
		// prepare persistent structures
		reshuffle();
		for (char ch = 'A'; ch <= 'Z'; ch++)
		{
			number[ch - 'A'].setFont(font);
			number[ch - 'A'].setCharacterSize(PPB / 4.0);
			if (tiles[ch - 'A'].size() > 0)
			{
				single.push_back(tiles[ch - 'A'][0]);
				stringstream str;
				str << tiles[ch - 'A'].size();
				number[ch - 'A'].setString(str.str());
			}
			number[ch - 'A'].setColor(sf::Color::Black);
			for (auto tile: tiles[ch - 'A'])
				sort.push_back(tile);
		}
	}

	void add_tile(Tile* tile)
	{
		reposition = true;
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

	void remove_tile(Tile* tile)
	{
		reposition = true;
		// update persistent structures
		scram.remove(tile);
		// Note: check for size 1 since the tile is removed *after* this function call
		if (tiles[tile->ch() - 'A'].size() == 1)
			single.remove(tile);
		else
		{
			stringstream str;
			str << (tiles[tile->ch() - 'A'].size() - 1);
			number[tile->ch() - 'A'].setString(str.str());
		}
		sort.remove(tile);
	}

	virtual bool process_event(const sf::Event& event)
	{
		auto prev = draw_func;
		if (event.type == sf::Event::KeyPressed)
		{
			switch (event.key.code)
			{
				case sf::Keyboard::F1:
					if (draw_func == &TileDisplay::scrambled)
						reshuffle();
					else
						draw_func = &TileDisplay::scrambled;
					break;
				case sf::Keyboard::F2:
					draw_func = &TileDisplay::ordered;
					break;
				case sf::Keyboard::F3:
					draw_func = &TileDisplay::counts;
					break;
				case sf::Keyboard::F4:
					draw_func = &TileDisplay::stacks;
					break;
				default:
					break;
			}
		}
		// reposition tiles if drawing function changed
		if (prev != draw_func)
			reposition = true;
		return true;
	}

	void draw()
	{
		auto view = window->getView();
		window->setView(window->getDefaultView());
		(this->*draw_func)();
		window->setView(view);
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

	void age(float time)
	{
		lifetime += time;
	}

	float age()
	{
		return lifetime;
	}

	void set_pos(float x, float y)
	{
		message.setPosition(x, y);
	}

	void draw_on(sf::RenderWindow& window) const
	{
		window.draw(message);
	}
};

class MessageQueue
{
	list<Message*> messages;
	sf::Font font;
public:
	enum severity_t {LOW, HIGH};

	MessageQueue(const sf::Font& f) : font(f)
	{
	}

	void add(const string& message, severity_t severity)
	{
		sf::Color color = sf::Color::Black;
		unsigned int size = 12;
		switch (severity)
		{
			case LOW:
				color = sf::Color::White;
				break;
			case HIGH:
				color = sf::Color::Red;
		}
		messages.push_back(new Message(message, font, size, color));
		// TODO bleh hardcoded
		messages.back()->set_pos(12, (messages.size() - 1) * 18 + 12);
	}

	void age(float time)
	{
		bool change = false;
		for (auto mess = messages.begin(); mess != messages.end();)
		{
			(*mess)->age(time);
			if ((*mess)->age() > 10)
			{
				mess = messages.erase(mess);
				change = true;
			}
			else
				mess++;
		}
		if (change)
		{
			unsigned int i = 12;
			for (auto message : messages)
			{
				message->set_pos(12, i);
				i += 18;
			}
		}
	}

	void draw_on(sf::RenderWindow& window) const
	{
		auto view = window.getView();
		window.setView(window.getDefaultView());
		for (auto message : messages)
			message->draw_on(window);
		window.setView(view);
	}
};

int main()
{
	std::srand((unsigned int)std::time(nullptr));

	sf::Font font;
	font.loadFromFile("C:\\Windows\\Fonts\\Vera.ttf");
	Grid grid;

	MessageQueue messages(font);

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
	// TODO validate somehow
	std::ifstream words("dictionary.txt");

	std::list<Tile*> bunch;

	sf::Color background(22, 22, 22);

	unsigned int res[2] = {1920, 1080};


	sf::RenderWindow window(sf::VideoMode(res[0], res[1]), "Bananagrams", sf::Style::Titlebar);
	window.setVerticalSyncEnabled(true);
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

	vector<InputReader*> input_readers;

	Game game(&window);
	input_readers.push_back(&game);

	vector<Tile*> tiles[26];
	TileDisplay display(&window, tiles, font);
	input_readers.push_back(&display);

	char ch = 'A' - 1;
	bool zoom_key = false;
	bool sprint_key = false;
	bool backspace = false;
	bool peel = false;
	int next[2];
	int last[2] = {0, 0};
	VimControls controls(delta, &ch, &zoom_key, &backspace, &grid, &peel);
	input_readers.push_back(&controls);

	clock.restart();

	// loading stuff
	bool loading = true;
	bool loading_words = true;
	char load_char = 'A';
	float miny;
	float height;
	bool done_y = false;
	string loading_str;
	sf::Text loading_text("", font, 30);

	while (window.isOpen())
	{
		if (loading)
		{
			if (loading_words)
			{
				string line;
				string word;
				float elapsed = 0;

				// TODO there must be a better way to get just the right speed...
				while (elapsed < 0.03)
				{
					if (std::getline(words, line))
					{
						auto pos = line.find_first_of(' ');
						if (pos == string::npos)
						{
							dictionary[line] = "";
							loading_text.setString(line);
						}
						else
						{
							dictionary[line.substr(0, pos)] = line.substr(pos + 1, string::npos);
							loading_text.setString(line.substr(0, pos));
						}
						loading_text.setPosition(loading_text.getGlobalBounds().width / -2, 0);

						elapsed = clock.getElapsedTime().asSeconds();
					}
					else
					{
						loading_words = false;
						stringstream foo;
						foo << dictionary.size();
						loading_text.setString(foo.str() + " words loaded.");
						loading_text.move(loading_text.getGlobalBounds().width / -2, -90);
						break;
					}
				}

				clock.restart();

				window.clear(background);
				window.draw(loading_text);
			}
			else
			{
				// TODO some kind of depth would help for stack appearance
				// TODO antialiase these
				// TODO error check
				// TODO memory leak here?
				tile_texture[load_char - 'A'].create(PPB, PPB);

				// draw character for centering
				stringstream string;
				string << load_char;
				sf::Text letter(string.str(), font, (PPB * 2) / 3.0);
				letter.setColor(sf::Color::Black);

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
				// draw final tile
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

				// create tiles
				for (unsigned int i = 0; i < letter_count[load_char - 'A']; i++)
				{
					auto it = bunch.begin();
					auto pos = std::rand() % (bunch.size() + 1);
					for (unsigned int i = 0; i != pos && it != bunch.end(); it++, i++);
					bunch.insert(it, new Tile(load_char));
				}

				sf::Sprite sprite(tile_texture[load_char - 'A'].getTexture());
				float padding = PPB / 2.0;
				sprite.setPosition(((load_char - 'A') * (res[0] - 2 * padding - PPB)) / 25.0 + padding - res[0] / 2, PPB / -2.0);
				window.draw(sprite);

				load_char++;
				if(load_char > 'Z')
				{
					for (unsigned int i = 0; i < 21; i++)
					{
						Tile* tile = bunch.back();
						bunch.pop_back();
						tiles[tile->ch() - 'A'].push_back(tile);
						display.add_tile(tile);
					}

					loading = false;
				}
			}
		}
		else
		{
			window.clear(background);
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

			if (peel)
			{
				bool spent = true;
				for (char ch = 'A'; ch <= 'Z'; ch++)
					if (tiles[ch - 'A'].size() > 0)
					{
						spent = false;
						break;
					}
				if (spent)
				{
					string mess;
					if (grid.is_valid(&mess))
					{
						if (bunch.size() > 0)
						{
							Tile* tile = bunch.back();
							// TODO it sucks that these don't sync automatically
							tiles[tile->ch() - 'A'].push_back(tile);
							display.add_tile(tile);
							bunch.pop_back();
						}
						else
							messages.add("You win!", MessageQueue::LOW);
					}
					else
						messages.add(mess, MessageQueue::HIGH);
				}
				else
					messages.add("You have not used all of your letters.", MessageQueue::HIGH);
				peel = false;
			}

			if (backspace)
			{
				backspace = false;

				// TODO DRY off autoadvancing
				// if the cursor is ahead of the last added character, autoadvance
				if (pos[0] == last[0] + next[0] && pos[1] == last[1] + next[1])
				{
					pos[0] = last[0];
					pos[1] = last[1];
					last[0] -= next[0];
					last[1] -= next[1];
				}
				// else if you are not near the last character and the space is empty, try to autoadvance
				else if (grid.get(pos[0], pos[1]) == nullptr)
				{
					if (grid.get(pos[0] - 1, pos[1]) != nullptr)
					{
						next[0] = 1;
						next[1] = 0;
						pos[0] -= next[0];
						last[0] = pos[0] - next[0];
						last[1] = pos[1];
					}
					else if (grid.get(pos[0], pos[1] - 1) != nullptr)
					{
						next[0] = 0;
						next[1] = 1;
						pos[1] -= next[1];
						last[0] = pos[0];
						last[1] = pos[1] - next[1];
					}
				}
				else // not near last character, position not empty, try to set autoadvance for next time
				{
					if (grid.get(pos[0] - 1, pos[1]) != nullptr)
					{
						next[0] = 1;
						next[1] = 0;
						last[0] = pos[0] - next[0];
						last[1] = pos[1];
					}
					else if (grid.get(pos[0], pos[1] - 1) != nullptr)
					{
						next[0] = 0;
						next[1] = 1;
						last[0] = pos[0];
						last[1] = pos[1] - next[1];
					}
				}

				auto tile = grid.remove(pos[0], pos[1]);
				if (tile != nullptr)
				{
					tiles[tile->ch() - 'A'].push_back(tile);
					display.add_tile(tile);
				}
			}
			else if (ch >= 'A' && ch <= 'Z')
			{
				bool placed = false;

				// if space is empty or has a different letter
				if (grid.get(pos[0], pos[1]) == nullptr || grid.get(pos[0], pos[1])->ch() != ch)
				{
					if (tiles[ch - 'A'].size() > 0)
					{
						Tile* tile = grid.swap(pos[0], pos[1], tiles[ch - 'A'].back());
						display.remove_tile(tiles[ch - 'A'].back());
						tiles[ch - 'A'].pop_back();
						if (tile != nullptr)
						{
							tiles[tile->ch() - 'A'].push_back(tile);
							display.add_tile(tile);
						}
						placed = true;
					}
				}
				else // space already has the letter to be placed
					placed = true;

				// if we placed a letter, try to autoadvance
				if (placed)
				{
					next[0] = 0;
					next[1] = 0;
					if (pos[0] == last[0] + 1 && pos[1] == last[1])
						next[0] = 1;
					else if (pos[0] == last[0] && pos[1] == last[1] + 1)
						next[1] = 1;
					else if (grid.get(pos[0] - 1, pos[1]) != nullptr)
						next[0] = 1;
					else if (grid.get(pos[0], pos[1] - 1) != nullptr)
						next[1] = 1;
					last[0] = pos[0];
					last[1] = pos[1];
					pos[0] += next[0];
					pos[1] += next[1];
				}

				// clear character to place
				ch = 'A' - 1;
			}

			float time = clock.getElapsedTime().asSeconds();
			clock.restart();

			messages.age(time);

			if (zoom_key)
				view.zoom(1 + delta[1] * (sprint_key ? 2 : 1) * time);
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
			// TODO view moves too slowly. Perhaps scale speed by how far cursor is from center?
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
			if (size.x < res[0] || size.y < res[1])
				view.setSize(res[0], res[1]);
			window.setView(view);

			cursor.setPosition(pos[0] * PPB + cursor_thickness, pos[1] * PPB + cursor_thickness);

			grid.draw_on(window);
			window.draw(cursor);

			display.draw();
			messages.draw_on(window);
		}

		window.display();
	}

	// delete unused tiles
	for (auto tile: bunch)
		delete tile;

	for (char ch = 'A'; ch <= 'Z'; ch++)
		for (auto tile: tiles[ch - 'A'])
			delete tile;

	return 0;
}

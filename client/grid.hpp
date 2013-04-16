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
	std::vector<Tile*> grid;
	unsigned int tiles {0};
	sf::Vector2i min {0, 0};
	sf::Vector2i max {0, 0};
	sf::Vector2i last;
	std::vector<std::string> defined;
	std::map<sf::Vector2i, bool> hwords;
	std::map<sf::Vector2i, bool> vwords;

	// for converting from 2D coords to vector positions
	inline unsigned int bijection(unsigned x, unsigned int y) const
	{
		return ((x + y) * (x + y + 1)) / 2 + x;
	}

	unsigned int convert(int x, int y) const;
	// for checking connectedness of grid
	void traverse(int x, int y);
public:
	~Grid();

	// return center of bounding box
	sf::Vector2f get_center() const;

	// return tile at x, y
	Tile* get(int x, int y) const;
	// remove tile at x, y and return it
	Tile* remove(int x, int y);
	// exchange tile at x, y for the given one, and return it
	Tile* swap(int x, int y, Tile* tile);
	// sf::Vector2i versions of the previous
	inline Tile* get(const sf::Vector2i& pos) const
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

	inline const sf::Vector2i& get_min() const
	{
		return min;
	}

	inline const sf::Vector2i& get_max() const
	{
		return max;
	}

	// remove all tiles
	void clear();

	// animate tiles
	void step(float time);

	// check for connectedness and valid words, chance of displaying definitions of valid words
	bool is_valid(std::map<std::string, std::string>& dictionary, std::vector<std::string>& messages, unsigned int chance = 100);

	void draw_on(sf::RenderWindow& window) const;
};

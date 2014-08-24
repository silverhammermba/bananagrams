// for single player game
class Bunch
{
protected:
	std::mt19937 rng;
public:
	Bunch();
	virtual ~Bunch() {}
	virtual unsigned int size() const = 0;
	virtual void add_tile(Tile* tile) = 0;
	virtual Tile* get_tile() = 0;
};

class FiniteBunch : public Bunch
{
	std::list<Tile*> tiles;
public:
	// numerator/denominator of bunch multipler, optional pointer to array of letter counts in play
	FiniteBunch(uint8_t num, uint8_t den, unsigned int* counts = nullptr);
	~FiniteBunch();

	unsigned int size() const;
	void add_tile(Tile* tile);
	Tile* get_tile();
};

class InfiniteBunch : public Bunch
{
	std::discrete_distribution<> dist;
public:
	InfiniteBunch();

	unsigned int size() const;
	void add_tile(Tile* tile);
	Tile* get_tile();
};

// for single player game
class Bunch
{
public:
	virtual ~Bunch() {}
	virtual unsigned int size() const = 0;
	virtual void add_tile(Tile* tile) = 0;
	virtual Tile* get_tile() = 0;
};

class FiniteBunch : public Bunch
{
	std::list<Tile*> tiles;
public:
	FiniteBunch(uint8_t num, uint8_t den, unsigned int* counts = nullptr);
	~FiniteBunch();

	unsigned int size() const;
	void add_tile(Tile* tile);
	Tile* get_tile();
};

/* TODO
class InfiniteBunch : public Bunch
{
	InfiniteBunch();
}
*/

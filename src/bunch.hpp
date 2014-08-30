#ifndef BUNCH_HPP
#define BUNCH_HPP

#include <list>
#include <random>

static const unsigned int letter_count[26]
{
    13, 3, 3, 6, 18, 3, 4, 3, 12, 2, 2, 5, 3, 8, 11, 3, 2, 9, 6, 9, 6, 3, 3, 2, 3, 2
//   A  B  C  D   E  F  G  H   I  J  K  L  M  N   O  P  Q  R  S  T  U  V  W  X  Y  Z
};

// for single player game
class Bunch
{
protected:
	std::mt19937 rng;
public:
	Bunch();
	virtual ~Bunch() {}
	virtual unsigned int size() const = 0;
	virtual void add_tile(char tile) = 0;
	virtual char get_tile() = 0;
};

class FiniteBunch : public Bunch
{
	std::list<char> tiles;
public:
	// numerator/denominator of bunch multipler, optional pointer to array of letter counts in play
	FiniteBunch(uint8_t num, uint8_t den, unsigned int* counts = nullptr);

	unsigned int size() const;
	void add_tile(char ch);
	char get_tile();
};

class InfiniteBunch : public Bunch
{
	std::discrete_distribution<> dist;
public:
	InfiniteBunch();

	unsigned int size() const;
	void add_tile(char tile);
	char get_tile();
};

#endif

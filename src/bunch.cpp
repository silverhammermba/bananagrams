#include "bunch.hpp"

Bunch::Bunch() : rng(std::random_device()())
{
}

FiniteBunch::FiniteBunch(uint8_t num, uint8_t den, unsigned int* counts)
{
	for (char ch = 'A'; ch <= 'Z'; ++ch)
		for (unsigned int i = 0; i < ((letter_count[ch - 'A'] * num) / den) - (counts == nullptr ? 0 : counts[ch - 'A']); ++i)
			add_tile(ch);
}

unsigned int FiniteBunch::size() const
{
	return tiles.size();
}

void FiniteBunch::add_tile(char ch)
{
	auto pos = std::uniform_int_distribution<>(0, tiles.size())(rng);

	auto it = tiles.begin();
	for (int i = 0; i != pos; ++i, ++it);
	tiles.insert(it, ch);
}

char FiniteBunch::get_tile()
{
	if (tiles.size() == 0) return 'A' - 1; // XXX make sure this doesn't happen!
	char ch = tiles.back();
	tiles.pop_back();
	return ch;
}

InfiniteBunch::InfiniteBunch()
	: dist(std::begin(letter_count), std::end(letter_count))
{
}

unsigned int InfiniteBunch::size() const
{
	return std::numeric_limits<unsigned int>::max();
}

void InfiniteBunch::add_tile(char ch)
{
	(void)ch; // intentionally unused
}

char InfiniteBunch::get_tile()
{
	return 'A' + dist(rng);
}

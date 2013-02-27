#ifndef BANANAGRAMS_HPP
#define BANANAGRAMS_HPP

static const int PPB = 48;
static const sf::Vector2i X(1, 0);
static const sf::Vector2i Y(0, 1);
static const sf::Vector2i XY(1, 1);

extern sf::RenderTexture tile_texture[26];
extern std::map<std::string, std::string> dictionary;

// TODO better place for this?
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

#endif

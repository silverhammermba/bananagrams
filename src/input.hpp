#ifndef INPUT_HPP
#define INPUT_HPP

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

	void reset()
	{
		finished = false;
	}

	virtual bool process_event(sf::Event& event)
	{
		(void)event; // intentionally unused parameter
		return true;
	}
};

#endif

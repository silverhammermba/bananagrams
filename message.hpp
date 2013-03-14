class Message
{
	sf::Text message;
	float lifetime = 0;
public:
	Message(const std::string& mes, const sf::Font& font, unsigned int size = 20, const sf::Color& color = sf::Color::Black);

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

// for managing and displaying a queue of messages
class MessageQ
{
	std::list<Message*> messages;
	sf::Font font;
	float bottom;
	static constexpr float padding = 10;
public:
	enum severity_t {LOW, HIGH};

	MessageQ(const sf::Font& f);
	~MessageQ();

	void add(const std::string& message, severity_t severity);
	void age(float time); // TODO inefficient?

	void draw_on(sf::RenderWindow& window) const;
};

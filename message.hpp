class Message
{
	sf::Text message;
	float lifetime {0};
public:
	enum class Severity {LOW, HIGH};

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
	float bottom {0};
	static constexpr float padding = 10;
public:
	MessageQ(const sf::Font& f);
	~MessageQ();

	void add(const std::string& message, Message::Severity severity);
	void age(float time); // TODO inefficient?
	void clear();

	void draw_on(sf::RenderWindow& window) const;
};

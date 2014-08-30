class Message
{
	sf::Text message;
	float age {0};
	float duration;
public:
	enum class Severity {LOW, HIGH, CRITICAL};

	Message(const std::string& mes, const sf::Font& font, unsigned int size = 20, const sf::Color& color = sf::Color::Black, float dur = 5);

	inline void step(float time)
	{
		age += time;
	}

	inline float can_be_removed() const
	{
		return duration > 0 && age > duration;
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
	void step(float time);
	void clear();

	void draw_on(sf::RenderWindow& window) const;
};

// TODO use inheritance to make single/multiplayer
class Game
{
	CutBuffer* buffer {nullptr};
	sf::Vector2i last_place {-1, 0};
	sf::Vector2i last_move {0, 0};
	// mouse press and release positions
	sf::Vector2i sel1;
	sf::Vector2i sel2;
	bool selecting {false};
	bool selected {false};
	bool set_view_to_cursor {true};
	Cursor mcursor {{1, 1}, PPB / 16.0, sf::Color::Transparent, sf::Color {0, 200, 0, 80}};
	Cursor selection {{1, 1}, 1, sf::Color {255, 255, 255, 25}, sf::Color::White};
protected:
	Grid grid;
	Hand hand {font};
	MessageQ messages {font};
	Cursor cursor {{1, 1}, PPB / 16.f, sf::Color::Transparent, sf::Color {0, 200, 0}};
public:
	virtual ~Game();

	inline Hand& get_hand()
	{
		return hand;
	}

	inline void step(float time)
	{
		grid.step(time);
		messages.step(time);
	}

	inline sf::Vector2f get_grid_center() const
	{
		return grid.get_center();
	}

	void clear_buffer();
	void update_mouse_pos(const sf::RenderWindow& window, const sf::View& view, const sf::Vector2i& pos);
	void select();
	inline bool is_selecting() const
	{
		return selecting;
	}
	void resize_selection();
	unsigned int complete_selection();
	void quick_place();
	void cut();
	void flip_buffer();
	void paste();
	void remove_at_mouse();
	virtual void dump() = 0;
	virtual bool word_is_valid(const std::string& word) const = 0;
	virtual bool peel();
	void remove();
	void place(char ch);
	inline void move_cursor(const sf::Vector2i& delta)
	{
		if (delta.y == 0 && delta.x != 0)
			last_move = X;
		else if (delta.x == 0 && delta.y != 0)
			last_move = Y;
		cursor.move(delta);
	}
	inline void set_cursor_to_view()
	{
		set_view_to_cursor = false;
	}
	void update_cursor(const sf::View& view);
	inline sf::Vector2f get_cursor_center()
	{
		return cursor.get_center();
	}
	void set_zoom(float zoom);
	void draw_on(sf::RenderWindow& window, const sf::View& grid_view, const sf::View& gui_view);
};

class SingleplayerGame : public Game
{
	std::string dict_str {""};
	std::map<std::string, std::string> dictionary;
	std::list<Tile*> bunch;
public:
	SingleplayerGame(const std::string& dict, int multiplier = 1, int divider = 1);
	virtual ~SingleplayerGame();

	bool load(const std::string& filename);
	void save(const std::string& filename);

	virtual void dump();
	virtual bool word_is_valid(const std::string& word) const;
	virtual bool peel();
};

class MultiplayerGame : public Game
{
	sf::UdpSocket socket;
	sf::IpAddress server_ip;
	unsigned short server_port;
	std::string id;
public:
	MultiplayerGame(const std::string& ip, unsigned short port, const std::string& name);
	virtual ~MultiplayerGame();

	virtual void dump();
	virtual bool word_is_valid(const std::string& word) const;
	virtual bool peel();
};

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
	bool playing;
	Grid grid;
	Hand hand {font};
	MessageQ messages {font};
	Cursor cursor {{1, 1}, PPB / 16.f, sf::Color::Transparent, sf::Color {0, 200, 0}};

	SoundManager& sound;

public:
	Game(bool _playing, SoundManager& _sound);
	virtual ~Game();

	inline bool in_progress() const
	{
		return playing;
	}

	inline Hand& get_hand()
	{
		return hand;
	}

	virtual void step(float time);

	inline sf::Vector2f get_grid_center() const
	{
		return grid.get_center();
	}

	virtual void ready() = 0;
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
	void prompt_show();
	void show(char ch);
	void prompt_dump();
	virtual void dump(char ch) = 0;
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
	virtual void draw_on(sf::RenderWindow& window, const sf::View& grid_view, const sf::View& gui_view) const;

	inline void split_sound()
	{
		sound.play("audio/split.wav");
	}
};

class SingleplayerGame : public Game
{
	std::string dict_filename;
	// you really shouldn't need more than 36,720 letters with a finite bunch
	uint8_t num;
	uint8_t den;

	std::map<std::string, std::string> dictionary;
	std::map<std::string, bool> spelled;
	Bunch* bunch;

	std::mt19937 rng;
public:
	SingleplayerGame(SoundManager& _sound, const std::string& dict, uint8_t _num = 1, uint8_t _den = 1);
	SingleplayerGame(SoundManager& _sound, std::ifstream& save_file);
	virtual ~SingleplayerGame();

	void save(const std::string& filename);

	virtual void ready() {};
	virtual void dump(char ch);
	virtual bool peel();
};

class MultiplayerGame : public Game
{
	gridword_map lookup_words;
	gridword_map bad_words;

	sf::UdpSocket socket;
	sf::IpAddress server_ip;
	unsigned short server_port;
	float time_stale;
	float poll_pause;
	float timeout;
	float polling;

	std::string id;
	bool connected = false;
	bool is_ready = false;
	bool waiting = false;
	sf::Int16 peel_n {-1};
	std::map<std::string, bool> dictionary;

	sf::Int16 dump_n {-1};

	sf::Packet* pending = nullptr;
	sf::Uint8 pending_type = 255;
	sf::Int16 ack_num {0};

	std::map<std::string, Player> players;
public:
	MultiplayerGame(SoundManager& _sound, const std::string& server, const std::string& name);
	virtual ~MultiplayerGame();

	// create new pending packet and store type
	void set_pending(sf::Uint8 type);
	// when a packet is acknowledged, reset packet sending state
	void clear_pending(bool force = false);
	// send unacknowledged packet
	void send_pending();

	void disconnect();

	virtual void step(float time);
	virtual void ready();
	virtual void dump(char ch);
	virtual bool peel();
	bool resolve_peel();

	void process_packet(sf::Packet& packet);
	virtual void draw_on(sf::RenderWindow& window, const sf::View& grid_view, const sf::View& gui_view) const;
};

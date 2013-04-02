class Game
{
	std::string dict_str {""};
	bool selecting {false};
	bool selected {false};
	bool set_view_to_cursor {true};
public:
	std::map<std::string, std::string> dictionary;
	std::list<Tile*> bunch;
	Grid grid;
	Hand hand {font};
	MessageQ messages {font};
	CutBuffer* buffer {nullptr};

	Cursor cursor {{1, 1}, PPB / 16.f, sf::Color::Transparent, sf::Color {0, 200, 0}};
	Cursor mcursor {{1, 1}, PPB / 16.0, sf::Color::Transparent, sf::Color {0, 200, 0, 80}};

	sf::Vector2i last {-1, 0};
	sf::Vector2i next {0, 0};

	// mouse press and release positions
	sf::Vector2i sel1;
	sf::Vector2i sel2;
	Cursor selection {{1, 1}, 1, sf::Color {255, 255, 255, 25}, sf::Color::White};

	~Game();


	bool load(const std::string& filename);
	void save(const std::string& filename);

	void clear_buffer();
	void end();
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
	void dump();
	void peel();
	void remove();
	void place(char ch);
	inline void move_cursor(const sf::Vector2i& delta)
	{
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

	bool restart(const std::string& dict, int multiplier = 1, int divider = 1);
};

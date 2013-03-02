// structs for passing around state
struct State
{
	// general
	sf::RenderWindow* window;
	sf::View* gui_view;
	sf::View* grid_view;
	float zoom; // zoom factor for grid view
	bool switch_controls; // signal to switch control schemes
	bool transpose; // flip selection
	bool center; // center grid

	// keyboard
	sf::Vector2i delta; // cursor movement signal
	char ch; // tile to place
	bool ctrl; // if Ctrl is being held
	bool sprint; // if cursor movement should be fast
	bool kremove; // signal to remove a tile
	bool peel;
	bool dump;
	bool cut;
	bool paste;

	// mouse
	int pos[2]; // last position
	bool update; // signal to update cursor position
	bool mremove; // signal to remove tiles
	int wheel_delta; // amount to zoom
	bool start_selection;
	bool end_selection;
};

class MouseControls : public InputReader
{
	State* state;
public:
	MouseControls(State* m);

	virtual bool process_event(const sf::Event& event);
};

class SimpleControls : public InputReader
{
	State* state;
public:
	SimpleControls(State* s);

	virtual bool process_event(const sf::Event& event);
};

class VimControls : public InputReader
{
	bool shift = false;
	State* state;
public:
	VimControls(State* s);

	virtual bool process_event(const sf::Event& event);
};

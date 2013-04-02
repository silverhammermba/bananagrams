class Game
{
public:
	std::map<std::string, std::string> dictionary;
	std::list<Tile*> bunch;
	Grid grid;
	Hand hand;
	MessageQ messages;
	CutBuffer* buffer {nullptr};
	bool selected {false};
	bool selecting {false};

	void end();
// TODO decide visibility
	Game();
	~Game();

	void clear_buffer();

	bool load(const std::string& filename);
	void save(const std::string& filename);

	bool restart(const std::string& dict, int multiplier = 1, int divider = 1);
};

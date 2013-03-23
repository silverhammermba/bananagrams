class Game
{
public:
	std::map<std::string, std::string> dictionary;
	std::list<Tile*> bunch;
	Hand hand;
	MessageQ messages;
	CutBuffer* buffer;
	bool selected;
	bool selecting;

	void end();
// TODO decide visibility
	Game();
	~Game();

	void clear_buffer();

	bool load(const std::string& filename);
	void save(const std::string& filename);

	void restart(const std::string& dict, int multiplier = 1, int divider = 1);
};

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>
#include <map>
#include <random>
#include <string>
#include <unordered_map>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>

#include "buffer.hpp"
#include "cursor.hpp"
#include "player.hpp"
#include "message.hpp" // TODO remove?

class Client
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

	bool playing;
	Grid grid;
	Hand hand;
	MessageQ messages;
	Cursor cursor {{1, 1}, PPB / 16.f, sf::Color::Transparent, sf::Color {0, 200, 0}};

	// networking
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
	bool started = false;
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

	bool is_sp;
public:
	Client(const sf::Font& font, const sf::IpAddress& server, unsigned short port, const std::string& name, bool _is_sp);
	~Client();

	inline bool in_progress() const
	{
		return playing;
	}

	inline Hand& get_hand()
	{
		return hand;
	}

	void step(float time);

	inline sf::Vector2f get_grid_center() const
	{
		return grid.get_center();
	}

	void ready();
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
	void dump(char ch);
	bool peel();
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
	void draw_on(sf::RenderWindow& window, const sf::View& grid_view, const sf::View& gui_view) const;

	inline bool game_started()
	{
		bool temp = started;
		started = false;
		return temp;
	}

	// create new pending packet and store type
	void set_pending(sf::Uint8 type);
	// when a packet is acknowledged, reset packet sending state
	void clear_pending(bool force = false);
	// send unacknowledged packet
	void send_pending();

	void disconnect();

	bool resolve_peel();

	void process_packet(sf::Packet& packet);
};

#endif

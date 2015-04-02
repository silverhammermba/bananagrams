#ifndef SERVER_HPP
#define SERVER_HPP

#include <chrono>
#include <iostream> // TODO handle output in server main
#include <mutex>
#include <string>
#include <thread>

#include <SFML/Network.hpp>

#include "constants.hpp"
#include "game.hpp"

class Server
{
public:
	enum class Status {LOADING, RUNNING, ABORTED, DONE};

private:
	std::mutex shutdown_lock;
	bool shutdown_signal;

	std::mutex status_lock;
	Status status;

	std::thread thread;

	// function to be run in thread
	void start(unsigned short port, const std::string& _dict_filename, uint8_t _num, uint8_t _den, unsigned int _max_players);

	// for saving the game
	std::string dict_filename;
	uint8_t num;
	uint8_t den;
	unsigned int* counts = nullptr;
public:
	// just passes along params to thread
	Server(unsigned short port, const std::string& _dict_filename, uint8_t _num, uint8_t _den, unsigned int _max_players);
	// for single player
	Server(const std::string& _dict_filename, uint8_t _num, uint8_t _den);
	// load game
	Server(std::ifstream& save_file);
	// makes sure thread exits
	~Server();

	// tell server thread to return
	void shutdown();
	// block until thread returns
	Status block();

	inline const std::string& get_dict_filename() const
	{
		return dict_filename;
	}

	uint8_t get_num() const
	{
		return num;
	}

	uint8_t get_den() const
	{
		return den;
	}

	Status get_status();
};

#endif

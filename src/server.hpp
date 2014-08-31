#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <mutex>
#include <string>
#include <thread>

#include <SFML/Network.hpp>

#include "constants.hpp"
#include "game.hpp"

class Server
{
public:
	enum class Status {RUNNING, ABORTED, EXITED};

private:
	std::mutex shutdown_lock;
	bool shutdown_signal;

	std::mutex status_lock;
	Status status;

	std::thread thread;

	// function to be run in thread
	void start(unsigned short port, const std::string& _dict_filename, uint8_t _num, uint8_t _den, unsigned int _max_players);

public:
	Server(unsigned short port, const std::string& _dict_filename, uint8_t _num, uint8_t _den, unsigned int _max_players);
	~Server();

	// tell server thread to return
	void shutdown();
	// block until thread returns
	Status block();
};

#endif

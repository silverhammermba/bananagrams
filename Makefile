# TODO maybe try out pendantic/strict ANSI?
CXXFLAGS=-std=c++11 -Wall -Wextra -Wfatal-errors -ggdb -pg
CLIENT_SOURCE:=$(wildcard client/*.cpp)
SERVER_SOURCE:=$(wildcard server/*.cpp)

# TODO have switch to toggle between debug/optimized builds

all: bananagrams dedicated_server

bananagrams: $(patsubst %.cpp,%.o,$(CLIENT_SOURCE))
	$(CXX) $(CXXFLAGS) -o bananagrams $+ -lyaml-cpp -lsfml-audio -lsfml-graphics -lsfml-window -lsfml-network -lsfml-system

client/client.hpp: client/*.hpp common.hpp
	touch client/client.hpp

client/%.o: client/%.cpp client/client.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

dedicated_server: $(patsubst %.cpp,%.o,$(SERVER_SOURCE))
	$(CXX) $(CXXFLAGS) -o dedicated_server $+ -lboost_program_options -lsfml-network -lsfml-system

server/server.hpp: server/*.hpp common.hpp
	touch server/server.hpp

server/%.o: server/%.cpp server/server.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f client/*.o server/*.o bananagrams dedicated_server

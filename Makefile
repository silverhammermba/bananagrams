CXX=g++
# TODO maybe try out pendantic/strict ANSI?
CXXFLAGS=-std=c++0x -Wall -Wextra -Wfatal-errors -ggdb -pg
CLIENT_SOURCE:=$(wildcard client/*.cpp)
SERVER_SOURCE:=$(wildcard server/*.cpp)

# TODO have switch to toggle between debug/optimized builds

bananagrams: $(patsubst %.cpp,%.o,$(CLIENT_SOURCE))
	$(CXX) $(CXXFLAGS) -o bananagrams $+ -lyaml-cpp -lsfml-graphics -lsfml-window -lsfml-system

client/bananagrams.hpp: client/*.hpp
	touch client/bananagrams.hpp

client/%.o: client/%.cpp client/bananagrams.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

dedicated_server: $(patsubst %.cpp,%.o,$(SERVER_SOURCE))
	$(CXX) $(CXXFLAGS) -o dedicated_server $+ -lboost_program_options

server/%.o: server/%.cpp server/server.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f client/*.o server/*.o bananagrams

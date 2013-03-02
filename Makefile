CXX=g++
CXXFLAGS=-std=c++0x -Wall -Wextra -Wfatal-errors -ggdb

bananagrams: main.o tile.o hand.o grid.o buffer.o message.o control.o cursor.o
	$(CXX) $(CXXFLAGS) -o bananagrams main.o tile.o hand.o grid.o buffer.o message.o cursor.o control.o -lsfml-graphics -lsfml-window -lsfml-system

message.o: message.cpp message.hpp
	$(CXX) $(CXXFLAGS) -c message.cpp

cursor.o: cursor.cpp cursor.hpp bananagrams.hpp
	$(CXX) $(CXXFLAGS) -c cursor.cpp

control.o: control.cpp control.hpp bananagrams.hpp
	$(CXX) $(CXXFLAGS) -c control.cpp

tile.o: tile.cpp tile.hpp
	$(CXX) $(CXXFLAGS) -c tile.cpp

hand.o: hand.cpp hand.hpp tile.hpp
	$(CXX) $(CXXFLAGS) -c hand.cpp

grid.o: grid.cpp grid.hpp tile.hpp
	$(CXX) $(CXXFLAGS) -c grid.cpp

buffer.o: buffer.cpp buffer.hpp bananagrams.hpp tile.hpp grid.hpp hand.hpp
	$(CXX) $(CXXFLAGS) -c buffer.cpp

main.o: main.cpp bananagrams.hpp tile.hpp grid.hpp hand.hpp buffer.hpp
	$(CXX) $(CXXFLAGS) -c main.cpp

clean:
	rm -f *.o bananagrams

CXX=g++
CXXFLAGS=-std=c++0x -Wall -Wextra -Wfatal-errors -ggdb

bananagrams: main.o tile.o hand.o grid.o buffer.o message.o control.o cursor.o
	$(CXX) $(CXXFLAGS) -o bananagrams main.o tile.o hand.o grid.o buffer.o message.o cursor.o control.o -lsfml-graphics -lsfml-window -lsfml-system

bananagrams.hpp: control.hpp grid.hpp tile.hpp buffer.hpp cursor.hpp hand.hpp message.hpp
	touch bananagrams.hpp

message.o: message.cpp bananagrams.hpp
	$(CXX) $(CXXFLAGS) -c message.cpp

cursor.o: cursor.cpp bananagrams.hpp
	$(CXX) $(CXXFLAGS) -c cursor.cpp

control.o: control.cpp bananagrams.hpp
	$(CXX) $(CXXFLAGS) -c control.cpp

tile.o: tile.cpp bananagrams.hpp
	$(CXX) $(CXXFLAGS) -c tile.cpp

hand.o: hand.cpp bananagrams.hpp
	$(CXX) $(CXXFLAGS) -c hand.cpp

grid.o: grid.cpp bananagrams.hpp
	$(CXX) $(CXXFLAGS) -c grid.cpp

buffer.o: buffer.cpp bananagrams.hpp
	$(CXX) $(CXXFLAGS) -c buffer.cpp

main.o: main.cpp bananagrams.hpp
	$(CXX) $(CXXFLAGS) -c main.cpp

clean:
	rm -f *.o bananagrams

CXX=g++
CXXFLAGS=-std=c++0x -Wall -Wextra -Wfatal-errors -ggdb -pg
SOURCE:=$(wildcard *.cpp)

bananagrams: $(patsubst %.cpp,%.o,$(SOURCE))
	$(CXX) $(CXXFLAGS) -o bananagrams $+ -lyaml-cpp -lsfml-graphics -lsfml-window -lsfml-system

bananagrams.hpp: *.hpp
	touch bananagrams.hpp

%.o: %.cpp bananagrams.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f *.o bananagrams

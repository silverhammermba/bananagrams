CXX=g++
# TODO maybe try out pendantic/strict ANSI?
CXXFLAGS=-std=c++0x -Wall -Wextra -Wfatal-errors -ggdb -pg
SOURCE:=$(wildcard *.cpp)

# TODO have switch to toggle between debug/optimized builds

bananagrams: $(patsubst %.cpp,%.o,$(SOURCE))
	$(CXX) $(CXXFLAGS) -o bananagrams $+ -lyaml-cpp -lsfml-graphics -lsfml-window -lsfml-system

bananagrams.hpp: *.hpp
	touch bananagrams.hpp

%.o: %.cpp bananagrams.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f *.o bananagrams

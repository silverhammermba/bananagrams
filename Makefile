bananagrams: main.cpp
	g++ -std=c++0x -Wall -Wextra -Wfatal-errors -ggdb main.cpp -o bananagrams -lsfml-graphics -lsfml-window -lsfml-system

clean:
	rm -f *.o bananagrams

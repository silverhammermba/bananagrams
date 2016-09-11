SOURCE:=$(wildcard ../src/*.cpp)

# TODO maybe try out pendantic/strict ANSI?
CXXFLAGS=-std=c++11
BOOST_PO=boost_program_options

ifdef RELEASE
CXXFLAGS+=-O2
else
CXXFLAGS+=-Wall -Wextra -Wfatal-errors -ggdb -pg
endif

ifdef WINDOWS
CXX=x86_64-w64-mingw32-g++
CXXFLAGS+=-static
BOOST_PO:=$(BOOST_PO)-mt
endif

all: $(CLIENT) $(SERVER)

depend: .depend

.depend: $(SOURCE) $(wildcard ../src/*.hpp)
	rm -f .depend
	$(CXX) $(CXXFLAGS) -MM $^ > .depend

include .depend

%.o: ../src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(CLIENT): client_main.o buffer.o bunch.o client.o control.o cursor.o game.o grid.o hand.o menu.o message.o player.o server.o tile.o
	$(CXX) $(CXXFLAGS) -o $(CLIENT) $^ -lyaml-cpp -lsfml-audio -lsfml-graphics -lsfml-window -lsfml-network -lsfml-system -pthread

$(SERVER): server_main.o bunch.o game.o player.o server.o
	$(CXX) $(CXXFLAGS) -o $(SERVER) $^ -l$(BOOST_PO) -lsfml-network -lsfml-system -pthread

clean:
	rm -f *.o $(CLIENT) $(SERVER)

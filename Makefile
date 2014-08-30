CLIENT=bananagrams
SERVER=dedicated_server
SOURCE:=$(wildcard src/*.cpp)
BINS=$(CLIENT) $(SERVER)

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
CLIENT:=$(CLIENT).exe
SERVER:=$(SERVER).exe
ZIP=bananagrams.zip
endif

all: $(BINS)

depend: .depend

.depend: $(SOURCE)
	rm -f .depend
	$(CXX) $(CXXFLAGS) -MM $^ > .depend

include .depend

$(CLIENT): src/client_main.o src/buffer.o src/bunch.o src/client.o src/control.o src/cursor.o src/game.o src/grid.o src/hand.o src/menu.o src/message.o src/player.o src/server.o
	$(CXX) $(CXXFLAGS) -o $(CLIENT) $^ -lyaml-cpp -lsfml-audio -lsfml-graphics -lsfml-window -lsfml-network -lsfml-system

$(SERVER): src/server_main.o src/bunch.o src/game.o src/player.o src/server.o
	$(CXX) $(CXXFLAGS) -o $(SERVER) $^ -l$(BOOST_PO) -lsfml-network -lsfml-system -pthread

clean:
	rm -f $(patsubst %.cpp,%.o,$(SOURCE)) $(BINS) $(ZIP)

dictionary.txt: words.txt
	ruby define.rb <words.txt >dictionary.txt

ifdef WINDOWS
$(ZIP): all dictionary.txt LICENSE.txt README.md audio/*
	rm -f $(ZIP)
	zip -ru $(ZIP) $(BINS) dictionary.txt LICENSE.txt README.md audio
	zip -ju $(ZIP) /usr/share/fonts/TTF/DejaVuSans.ttf
	ruby get_dlls.rb $(BINS) | zip -ju@ $(ZIP)
endif

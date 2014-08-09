CLIENT=bananagrams
SERVER=dedicated_server
CLIENT_SOURCE:=$(wildcard client/*.cpp)
SERVER_SOURCE:=$(wildcard server/*.cpp)
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
PKG=bananagrams.zip
endif

all: $(BINS)

$(CLIENT): $(patsubst %.cpp,%.o,$(CLIENT_SOURCE))
	$(CXX) $(CXXFLAGS) -o $(CLIENT) $+ -lyaml-cpp -lsfml-audio -lsfml-graphics -lsfml-window -lsfml-network -lsfml-system

client/client.hpp: client/*.hpp common.hpp
	touch client/client.hpp

client/%.o: client/%.cpp client/client.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(SERVER): $(patsubst %.cpp,%.o,$(SERVER_SOURCE))
	$(CXX) $(CXXFLAGS) -o $(SERVER) $+ -l$(BOOST_PO) -lsfml-network -lsfml-system

server/server.hpp: server/*.hpp common.hpp
	touch server/server.hpp

server/%.o: server/%.cpp server/server.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f client/*.o server/*.o $(BINS) $(PKG)

ifdef WINDOWS
pkg: all
	rm -f $(PKG)
	zip -ru $(PKG) $(BINS) words.txt audio
	zip -ju $(PKG) /usr/share/fonts/TTF/DejaVuSans.ttf
	ruby get_dlls.rb $(BINS) | zip -ju@ $(PKG)
	# lib has wrong name, not found by get_dlls
	cp /usr/x86_64-w64-mingw32/bin/libjpeg-8.dll libjpeg-62.dll
	zip -u $(PKG) libjpeg-62.dll
	rm libjpeg-62.dll

endif

CLIENT_SOURCE:=$(wildcard client/*.cpp)
SERVER_SOURCE:=$(wildcard server/*.cpp)

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
SUFFIX=.exe
endif

all: bananagrams$(SUFFIX) dedicated_server$(SUFFIX)

bananagrams$(SUFFIX): $(patsubst %.cpp,%.o,$(CLIENT_SOURCE))
	$(CXX) $(CXXFLAGS) -o bananagrams$(SUFFIX) $+ -lyaml-cpp -lsfml-audio -lsfml-graphics -lsfml-window -lsfml-network -lsfml-system

client/client.hpp: client/*.hpp common.hpp
	touch client/client.hpp

client/%.o: client/%.cpp client/client.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

dedicated_server$(SUFFIX): $(patsubst %.cpp,%.o,$(SERVER_SOURCE))
	$(CXX) $(CXXFLAGS) -o dedicated_server$(SUFFIX) $+ -l$(BOOST_PO) -lsfml-network -lsfml-system

server/server.hpp: server/*.hpp common.hpp
	touch server/server.hpp

server/%.o: server/%.cpp server/server.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f client/*.o server/*.o bananagrams bananagrams.exe dedicated_server dedicated_server.exe

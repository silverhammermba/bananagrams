CLIENT=bananagrams
SERVER=dedicated_server

ifdef WINDOWS
CLIENT:=$(CLIENT).exe
SERVER:=$(SERVER).exe
ZIP=bananagrams.zip
endif

export CLIENT
export SERVER

all:
	$(MAKE) -C build

clean:
	$(MAKE) -C build clean
	rm -f $(ZIP)

dictionary.txt: words.txt
	ruby define.rb <words.txt >dictionary.txt

ifdef WINDOWS
BINS=build/$(CLIENT) build/$(SERVER)
$(ZIP): all dictionary.txt LICENSE.txt README.md audio/*
	rm -f $(ZIP)
	zip -ru $(ZIP) dictionary.txt LICENSE.txt README.md audio
	zip -ju $(ZIP) $(BINS) /usr/share/fonts/TTF/DejaVuSans.ttf
	ruby get_dlls.rb $(BINS) | zip -ju@ $(ZIP)
endif

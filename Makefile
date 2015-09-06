# GameBoy Emulator Makefile

BIN = out/gameboy
INCLUDES = -I /usr/include/SDL

SOURCES = gameboy.c platform.c sound.c screen.c

MAKE_OPTIONS = -Wall -ansi -O3

LIBS =  -lm -lSDL

OPTIONS = $(LIBS) `sdl-config --cflags --libs`

all:
	g++ $(MAKE_OPTIONS) $(SOURCES) $(INCLUDES) -o $(BIN) $(OPTIONS)

clean:
	rm -rf $(BIN)

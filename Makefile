all:
	gcc gameboy.c platform.c sound.c screen.c -lm -lSDL -I /usr/include/SDL -o gameboy

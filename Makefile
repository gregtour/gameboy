all:
	gcc gameboy.c platform.c sound.c screen.c -lSDL -I /usr/include/SDL -o gameboy

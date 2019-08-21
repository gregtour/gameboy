#ifndef _FONT_H
#define _FONT_H

#include "SDL.h"

void FontStartup();
void FontPrint(SDL_Renderer* renderer, const char* text, int x, int y);

#define FONT_BITMAP_WIDTH   64
#define FONT_BITMAP_HEIGHT  64
#define FONT_CHAR_WIDTH     6
#define FONT_CHAR_HEIGHT    6
#define FONT_COLUMNS        10
#define FONT_RENDER_SCALE   2
#define TEXT_FOREGROUND     0xFFFFFFFF
#define TEXT_BACKGROUND     0x00000000

#endif

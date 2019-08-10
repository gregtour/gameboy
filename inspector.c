#include <string.h>
#include "inspector.h"
#include "gameboy.h"
#include "platform.h"
#include "font.h"

char text_temp[512];

#define PRINTN(N, REG, X, Y)	\
	do { 	\
		sprintf(text_temp, #REG ": %0" #N "X", (REG)); \
		FontPrint(renderer, text_temp, (X), (Y)); } \
	while (0)

#define PRINT4(REG, X, Y)		PRINTN(4, REG, X, Y)
#define PRINT(REG, X, Y)		PRINTN(2, REG, X, Y)

SDL_Surface* inspector_surface = NULL;

void InspectorStartup(SDL_Renderer* renderer)
	{
	//inspector_surface = SDL_CreateRGBSurface(0, WINDOW_WIDTH, WINDOW_HEIGHT, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
	FontStartup(renderer);
	}


void InspectorClick(int x, int y)
	{

	}

void InspectorDraw(SDL_Renderer* renderer)
	{
	SDL_Rect rectangle;

	// clear screen
	rectangle.x = 0; 	rectangle.y = 0;
	rectangle.w = 50; 	rectangle.h = 50;
	SDL_SetRenderDrawColor(renderer, 80, 0, 80, 255);
	SDL_RenderFillRect(renderer, NULL);

	// CPU registers	
	PRINT(R_A, 10, 10); 	PRINT(R_B, 130, 10);	PRINT(R_D, 250, 10);	PRINT(R_H, 370, 10);	PRINT4(PC, 490, 10);
	/*				 */ 	PRINT(R_C, 130, 30);	PRINT(R_E, 250, 30);	PRINT(R_L, 370, 30);	PRINT4(SP, 490, 30);

	// instructions
	u32 start = (PC / 0x10) * 0x10;
	for (u32 index = 0; index < 0x10; index++)
		{
		u32 inst_addr = start + index;
		u8 inst_value = READ(inst_addr);
		sprintf(text_temp, "%s%04X %02X", (inst_addr == PC) ? "> " : "  ", inst_addr, inst_value);
		FontPrint(renderer, text_temp, 466, 120 + (index * 20));
		}
	}

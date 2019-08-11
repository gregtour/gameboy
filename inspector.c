#include <string.h>
#include "inspector.h"
#include "gameboy.h"
#include "platform.h"
#include "font.h"

char text_temp[512];

#define PRINTN(N, STR, REG, X, Y)	\
	do { 	\
		sprintf(text_temp, STR ": %0" #N "X", (REG)); \
		FontPrint(renderer, text_temp, (X), (Y)); } \
	while (0)

#define PRINTR8(REG, X, Y)		PRINTN(8, #REG, REG, X, Y)
#define PRINTR4(REG, X, Y)		PRINTN(4, #REG, REG, X, Y)
#define PRINTR(REG, X, Y)		PRINTN(2, #REG, REG, X, Y)
#define PRINT4(STR, REG, X, Y)	PRINTN(4, STR, REG, X, Y)
#define PRINTI(STR, REG, X, Y)	PRINTN(2, STR, REG, X, Y)
#define PRINTS(STR, X, Y)		do { FontPrint(renderer, STR, X, Y); } while (0)

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
	u32 start, index;

	// clear screen
	rectangle.x = 0; 	rectangle.y = 0;
	rectangle.w = 50; 	rectangle.h = 50;
	SDL_SetRenderDrawColor(renderer, 40, 0, 80, 255);
	SDL_RenderFillRect(renderer, NULL);


	FontPrint(renderer, cgb_enable ? "CGB" : "DMG", 10, 10);
	FontPrint(renderer, cgb_double ? "DOUBLE" : "SINGLE", 70, 10);

	PRINTR(gb_halt, 250, 10);
	PRINTR(gb_ime, 430, 10);


	// CPU registers	
	PRINTR(R_A, 10, 30); 	PRINTR(R_B, 130, 30);	PRINTR(R_D, 250, 30);	PRINTR(R_H, 370, 30);	PRINTR4(PC, 490, 30);
	/*				 */ 	PRINTR(R_C, 130, 50);	PRINTR(R_E, 250, 50);	PRINTR(R_L, 370, 50);	PRINT4("_PC", prev_PC, 490, 50);
																									PRINTR4(SP, 490, 70);

	//PRINT("WRAM BANK", wram_bank, 10, 70);
	//PRINT("VRAM BANK", vram_bank, 10, 90);
	PRINTR(R_IF, 10, 70);	PRINTR(R_IE, 130, 70);

	PRINTR(R_P1, 10, 90);	PRINTR(R_DMA, 130, 90);

	PRINTR(R_HDMA, 10, 110);
	PRINTR4(R_HDMAS, 150, 110);
	PRINTR4(R_HDMAD, 330, 110);

	PRINTR(cram_bank, 10, 450);
	PRINTR(wram_bank, 10, 470);
	PRINTR(vram_bank, 10, 490);
	PRINTR8(cpu_count, 10, 510);

	// instruction memory
	const int inst_x = 372 - 12; //370;
	const int inst_y = 144;
	PRINTS(" INST", inst_x, inst_y-4);
	PRINTS(" ____", inst_x, inst_y);
	start = (PC / 0x10) * 0x10;
	for (index = 0; index < 0x10; index++)
		{
		u32 inst_addr = start + index;
		u8 inst_value = READ(inst_addr);
		sprintf(text_temp, "%s%04X %02X", (inst_addr == PC) ? "> " : "  ", inst_addr, inst_value);
		FontPrint(renderer, text_temp, inst_x, inst_y + (index + 1) * 20);
		}

	// stack memory
	const int stack_x = 500 - 12; //502;
	const int stack_y = 144;
	PRINTS(" STACK", stack_x, stack_y-4);
	PRINTS(" _____", stack_x, stack_y);
	start = (SP / 0x08) * 0x08;
	for (index = 0; index < 0x10; index++)
		{
		u32 stack_addr = start + index;
		u8 stack_value = READ(stack_addr);
		sprintf(text_temp, "%s%04X %02X", (stack_addr == SP) ? "> " : "  ", stack_addr, stack_value);
		FontPrint(renderer, text_temp, stack_x, stack_y + (index + 1) * 20);
		}
	}

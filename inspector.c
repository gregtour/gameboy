#include <string.h>
#include "inspector.h"
#include "gameboy.h"
#include "platform.h"
#include "font.h"
#include "sound.h"

char text_temp[512];

#define PRINTN(N, STR, REG, X, Y)   \
    do {    \
        sprintf(text_temp, STR ": %0" #N "X", (REG)); \
        FontPrint(renderer, text_temp, (X), (Y)); } \
    while (0)

#define PRINTR8(REG, X, Y)      PRINTN(8, #REG, REG, X, Y)
#define PRINTR4(REG, X, Y)      PRINTN(4, #REG, REG, X, Y)
#define PRINTR(REG, X, Y)       PRINTN(2, #REG, REG, X, Y)
#define PRINT4(STR, REG, X, Y)  PRINTN(4, STR, REG, X, Y)
#define PRINTI(STR, REG, X, Y)  PRINTN(2, STR, REG, X, Y)
#define PRINTS(STR, X, Y)       do { FontPrint(renderer, STR, X, Y); } while (0)

SDL_Surface* inspector_surface = NULL;

void InspectorStartup(SDL_Renderer* renderer)
{
    //inspector_surface = SDL_CreateRGBSurface(0, WINDOW_WIDTH, WINDOW_HEIGHT, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
    FontStartup(renderer);
}


void InspectorClick(int x, int y)
{

}

extern u8 READ_SO_50();
extern u8 READ_SO_51();
extern u8 READ_SO_52();


extern u32 cram_reads;
extern u32 cram_writes;


void InspectorDraw(SDL_Renderer* renderer)
{
    SDL_Rect rectangle;
    u32 start, index;

    // clear screen
    rectangle.x = 0;    rectangle.y = 0;
    rectangle.w = 50;   rectangle.h = 50;
    SDL_SetRenderDrawColor(renderer, 40, 0, 80, 255);
    SDL_RenderFillRect(renderer, NULL);


    FontPrint(renderer, cgb_enable ? "CGB" : "DMG", 10, 10);
    FontPrint(renderer, cgb_double ? "DOUBLE" : "SINGLE", 70, 10);

#if 0
    PRINTR(gb_halt, 250, 10);
    PRINTR(gb_ime, 430, 10);
#endif

    // CPU registers    
    PRINTR(R_A, 10, 30);    PRINTR(R_B, 130, 30);   PRINTR(R_D, 250, 30);   PRINTR(R_H, 370, 30);
    /*               */     PRINTR(R_C, 130, 50);   PRINTR(R_E, 250, 50);   PRINTR(R_L, 370, 50);

    PRINTR4(PC, 370, 90);
    PRINTR8(cpu_count, 370, 110);


    //PRINT("WRAM BANK", wram_bank, 10, 70);
    //PRINT("VRAM BANK", vram_bank, 10, 90);
#if 0
    PRINTR(R_IF, 10, 70);   PRINTR(R_IE, 130, 70);
    PRINTR(R_P1, 10, 90);   PRINTR(R_DMA, 130, 90);
#endif

#if 0
    PRINTR(R_HDMA, 10, 110);
    PRINTR4(R_HDMAS, 150, 110);
    PRINTR4(R_HDMAD, 330, 110);
#endif

    // Display MBC info
#if 0
    PRINTR(gb_mbc, 10, 460);
    PRINTR(cram_bank, 10, 480); PRINTR(cram_enable, 210, 480);
    PRINTR(cram_banks, 10, 500); PRINTR(rom_bank, 210, 500);
    PRINTR8(cram_reads, 10, 520); 
    PRINTR8(cram_writes, 10, 540);

    //PRINTI("MBC INFO", ROM[ROM_MBC_INFO], 10, 530); PRINTI("MBC BANKS", ROM[ROM_BANK_COUNT], 200, 530);
    //PRINTI("MBC RAM", ROM[ROM_RAM_SIZE], 10, 550);  PRINTI("ROM BANKS", rom_banks, 200, 550);
#endif 

//    PRINTR(vram_bank, 10, 90);
//    PRINTI("TILE SELECT", (R_LCDC & LCDC_TILE_SELECT), 10, 110);

#if 1 // display sound channel 1
    PRINTI("CH1 CH EN", CH1.channel.enable, 10, 480);
    PRINTI("CH1 CH COUNT", CH1.channel.counterset, 10, 500);
    PRINTI("CH1 CH TIMER", CH1.channel.timer, 10, 520);
    PRINTI("CH1 CH INIT", CH1.channel.initset, 10, 540);
    PRINT4("CH1 CH FREQ", CH1.channel.freq, 10, 560);

    PRINTI("CH1 DISABLE", CH1.envelope.disabled, 220, 480);
    PRINTI("CH1 VOL", CH1.envelope.volume, 220, 500);
    PRINTI("CH1 DIR", CH1.envelope.dir, 220, 520);
    PRINTI("CH1 PER", CH1.envelope.period, 220, 540);
    PRINT4("CH1 TIMER", CH1.envelope.timer, 220, 560);

    // PRINTI("CH1 DISABLE", CH1.envelope.disabled, 220, 480);
    // PRINTI("CH1 VOL", CH1.envelope.volume, 220, 500);
    // PRINTI("CH1 DIR", CH1.envelope.dir, 220, 520);
    // PRINTI("CH1 PER", CH1.envelope.period, 220, 540);
#endif

#if 0 // display sound channel 3

    PRINTI("CH3 EN", CH3.enable, 220, 480);
    PRINTI("CH3 LEN", CH3.sound_len, 220, 500);
    PRINTI("CH3 LEVEL", CH3.out_level, 220, 520);
    PRINTI("CH3 COUNTER", CH3.pos_counter, 220, 540);

    PRINTI("CH3 CH EN", CH3.channel.enable, 10, 480);
    PRINTI("CH3 CH COUNT", CH3.channel.counterset, 10, 500);
    PRINTI("CH3 CH TIMER", CH3.channel.timer, 10, 520);
    PRINTI("CH3 CH INIT", CH3.channel.initset, 10, 540);
#endif

#if 0 // display sound system
    u8 NR50 = READ_SO_50();
    u8 NR51 = READ_SO_51();
    u8 NR52 = READ_SO_52();

    PRINTR(NR50, 510, 500);
    PRINTR(NR51, 510, 520);
    PRINTR(NR52, 510, 540);
#endif 

// Display palette registers
#if 0
    PRINTS("BCPD", 370, 144);
    PRINTS("OCPD", 502, 144);
    for (index = 0; index < 0x10; index++)
    {
        u16 value;
        value = BCPD[index];
        sprintf(text_temp, "%02X %04X", index, value);
        FontPrint(renderer, text_temp, 370, 144 + (index + 1) * 20);

        value = OCPD[index];
        sprintf(text_temp, "%02X %04X", index, value);
        FontPrint(renderer, text_temp, 502, 144 + (index + 1) * 20);
    }
#endif

// Display stack and instructions
#if 1
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
#endif
}

/*************
 * gameboy.h * 
*************/

#ifndef _GAMEBOY_H
#define _GAMEBOY_H

#include <stdint.h>

//#define 	EMULATE_MBC5
//#define 	EMULATE_RTC

//#define CPU_CLOCKSPEED        0x400000
#define CPU_CLOCKSPEED      4000000

// types
typedef uint64_t    u64;
typedef uint32_t    u32;
typedef uint16_t    u16;
typedef uint8_t     u8;
typedef int8_t      s8;
typedef int16_t     s16;
typedef int32_t     s32;
typedef int64_t     s64;


#ifndef NULL
#define NULL 0L
#endif

#ifndef MIN
#define MIN(a, b)   ((a) < (b) ? (a) : (b))
#endif

void LoadROM(u8* rom, u32 size, u8* save, u32 save_size);
u32  GetSaveSize(u8* rom);
void RunFrame();
void KeyPress(u8 k);
void KeyRelease(u8 k);
void SetFrameSkip(u8 f);

extern u8 gb_framecount;
extern u8 gb_frameskip;
extern u8 cgb_enable;

#define DMG_BIOS_SIZE   0x0100
#define CGB_BIOS_PART   0x0200
#define CGB_BIOS_SIZE   0x0900

// bios
#define DMG_BIOS_ENABLE    // optional logo: requires dmg.c
#ifdef DMG_BIOS_ENABLE
    extern u8  DMG_BIOS[0x100];
#endif

// internal functions
void UpdateP1();
void PowerUp();
void StepCPU();
void ExecuteCB();
void LCDStart();
void LCDDrawLine();
void LCDDrawLineMono();
void LCDDrawLineColor();

// interrupts
#define VBLANK_INTR     0x01
#define LCDC_INTR       0x02
#define TIMER_INTR      0x04
#define SERIAL_INTR     0x08
#define CONTROL_INTR    0x10
#define ANY_INTR        0x1F

// interrupt jump addresses
#define VBLANK_INTR_ADDR    0x0040
#define LCDC_INTR_ADDR      0x0048
#define TIMER_INTR_ADDR     0x0050
#define SERIAL_INTR_ADDR    0x0058
#define CONTROL_INTR_ADDR   0x0060

// memory section sizes
// DMG
//#define WRAM_SIZE       0x2000
//#define VRAM_SIZE       0x2000
// CGB
#define WRAM_SIZE       0x8000
#define VRAM_SIZE       0x4000
#define HRAM_SIZE       0x0100
#define OAM_SIZE        0x00A0

// MEMORY ///////////////////////////
extern u8 WRAM[WRAM_SIZE];
extern u8 VRAM[VRAM_SIZE];
extern u8 HRAM[HRAM_SIZE];
extern u8 OAM[OAM_SIZE];

// flags: ZERO, NEGATIVE OP (1: SUB, 0: ADD), HALF CARRY, CARRY
#define ZF_FLAG         0x0080
#define N_FLAG          0x0040
#define H_FLAG          0x0020
#define CY_FLAG         0x0010

// cart section sizes
#define ROM_BANK_SIZE   0x4000
#define WRAM_BANK_SIZE  0x1000
#define CRAM_BANK_SIZE  0x2000
#define VRAM_BANK_SIZE  0x2000

// CGB support
#define GB_COLOR_SUPPORT
#define CGB_OPTIONAL    0x80
#define CGB_EXCLUSIVE   0xC0
//#define CGB_OPTIONAL    0x100
//#define CGB_EXCLUSIVE   0x100

// ROM info locations
#define ROM_CGB_SUPPORT 0x0143
#define ROM_MBC_INFO    0x0147
#define ROM_BANK_COUNT  0x0148
#define ROM_RAM_SIZE    0x0149

// addresses
#define ROM_0_ADDR      0x0000
#define ROM_N_ADDR      0x4000
#define VRAM_ADDR       0x8000
#define CART_RAM_ADDR   0xA000
#define WRAM_0_ADDR     0xC000
#define WRAM_1_ADDR     0xD000
#define ECHO_ADDR       0xE000
#define OAM_ADDR        0xFE00
#define UNUSED_ADDR     0xFEA0
#define IO_ADDR         0xFF00
#define HRAM_ADDR       0xFF00
#define INTR_EN_ADDR    0xFFFF

// MBC control
#define RAM_ENABLE_ADDR 0x0000
#define ROM_BANK_ADDR   0x2000
#define RAM_BANK_ADDR   0x4000
#define RAM_MODE_ADDR   0x6000

// DIV // 16328 Hz
#define DIV_CYCLES          256

// TAC
#define TAC_ENABLE          0x04
#define TAC_RATE            0x03
extern const u32 TAC_CYCLES[4];

// LCD
#define LCD_LINE_CYCLES     456
#define LCD_MODE_0_CYCLES   0
#define LCD_MODE_2_CYCLES   204
#define LCD_MODE_3_CYCLES   284
#define LCD_VERT_LINES      154
#define LCD_WIDTH           160
#define LCD_HEIGHT          144

// framebuffers
extern u8 gb_fb[LCD_HEIGHT][LCD_WIDTH];
extern u16 cgb_fb[LCD_HEIGHT][LCD_WIDTH];

// PALETTES
extern u8 BGP[4];
extern u8 OBJP[8];

// CGB PALETTES
extern u16 BCPD[4*8];
extern u16 OCPD[4*8];

// LCD modes
#define LCD_HBLANK          0x0
#define LCD_VBLANK          0x1
#define LCD_SEARCH_OAM      0x2
#define LCD_TRANSFER        0x3

// STAT register controls
#define STAT_LYC_INTR       0x40
#define STAT_MODE_2_INTR    0x20
#define STAT_MODE_1_INTR    0x10
#define STAT_MODE_0_INTR    0x08
#define STAT_LYC_COINC      0x04
#define STAT_MODE           0x03
#define STAT_USER_BITS      0xF8

// LCDC controls
#define LCDC_ENABLE         0x80
#define LCDC_WINDOW_MAP     0x40
#define LCDC_WINDOW_ENABLE  0x20
#define LCDC_TILE_SELECT    0x10
#define LCDC_BG_MAP         0x08
#define LCDC_OBJ_SIZE       0x04
#define LCDC_OBJ_ENABLE     0x02
#define LCDC_BG_ENABLE      0x01

// SPRITE controls
#define NUM_SPRITES         0x28
#define MAX_SPRITES_LINE    0x0A
#define OBJ_PRIORITY        0x80
#define OBJ_FLIP_Y          0x40
#define OBJ_FLIP_X          0x20
#define OBJ_PALETTE         0x10
// CGB SPRITE controls
#define OBJ_VRAM_BANK       0x08
#define OBJ_CPALETTE        0x07

// CGB BG Map Attributes
#define BMAP_PRIORITY       0x80
#define BMAP_FLIP_Y         0x40
#define BMAP_FLIP_X         0x20
#define BMAP_VRAM_BANK      0x08
#define BMAP_CPALETTE       0x07

// INPUT
#define INPUT_P1            0xFF00
#define INPUT_KEYS          0x0F
#define INPUT_P14           0x10
#define INPUT_P15           0x20

// VRAM Locations
#define VRAM_TILES_1        (0x8000 - VRAM_ADDR)
#define VRAM_TILES_2        (0x8800 - VRAM_ADDR)
#define VRAM_BMAP_1         (0x9800 - VRAM_ADDR)
#define VRAM_BMAP_2         (0x9C00 - VRAM_ADDR)
#define VRAM_TILES_3        (0x8000 - VRAM_ADDR + VRAM_BANK_SIZE)
#define VRAM_TILES_4        (0x8800 - VRAM_ADDR + VRAM_BANK_SIZE)

#define HDMA_OFF            0x80
#define HDMA_HBLANK         0x80
#define HDMA_LENGTH         0x7F

#define PAL_AUTO_INCREMENT  0x80
#define PAL_INDEX           0x3F
#define PAL_CONTROL_BITS    (PAL_AUTO_INCREMENT | PAL_INDEX)

// CPU SETTINGS
extern u8 gb_bios_enable;
extern u8 cgb_bios_enable;
extern u8 opt_use_gb_bios;
extern u8 gb_halt;
extern u8 gb_ime;
extern u8 gb_keys;
extern u8 gb_mbc;
extern u8 gb_cram;
extern u8 gb_frame;
extern u8 gb_rtc[5];
extern u8 lcd_mode;

// GB SETTINGS
extern u8 cgb_double;
extern u16 rom_bank;
extern u16 rom_banks;
extern u8  wram_bank;
extern u8  vram_bank;
extern u8  cram_bank;
extern u8  cram_banks;
extern u8  cram_enable;
extern u8  cram_mode;

extern u8* ROM;
extern u8* BIOS;
extern u32 BIOS_SIZE;

// TIMERS
extern u32 cpu_count;
extern u32 lcd_count;
extern u32 audio_count;
extern u32 div_count;
extern u32 tima_count;
extern u8  tac_enable;
extern u8  tac_rate;

// CPU REGISTERS
extern u8 R_A;
extern u8 R_B; extern u8 R_C;
extern u8 R_D; extern u8 R_E;
extern u8 R_H; extern u8 R_L;
extern u16 SP;
extern u16 PC;
extern u16 prev_PC;

// REGISTERS
extern u8 R_P1;    extern u8 R_SB;    extern u8 R_SC;    extern u8 R_DIV;
extern u8 R_TIMA;  extern u8 R_TMA;   extern u8 R_TAC;   extern u8 R_IF;
extern u8 R_LCDC;  extern u8 R_STAT;
extern u8 R_SCY;   extern u8 R_SCX;   extern u8 R_LY;    extern u8 R_LYC;
extern u8 R_DMA;   extern u8 R_BGP;   extern u8 R_OBP0;  extern u8 R_OBP1;
extern u8 R_WY;    extern u8 R_WX;    extern u8 R_IE;

// CGB REGISTERS
extern u8 R_HDMA;  extern u8 R_BCPS;  extern u8 R_OCPS;  extern u8 R_KEY1;
extern u16 R_HDMAS;
extern u16 R_HDMAD;


// MEMORY
u8 READ(u16 addr);

#endif // _GAMEBOY_H

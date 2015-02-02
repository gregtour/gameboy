/*************
 * gameboy.h * 
*************/

// types
typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;
typedef signed char s8;

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


// Instruction Mnuemonics
/*
const char* OP_MN[] = 
    { // 0x0
    "NOP",              "LD BC, imm",       "LD (BC), A",       "INC BC",
    "INC B",            "DEC B",            "LD B, imm",        "RLCA",
    "LD (imm), SP",     "ADD HL, BC",       "LD A, (BC)",       "DEC BC",
    "INC C",            "DEC C",            "LD C, imm",        "RRCA",
    // 0x1
    "STOP",             "LD DE, imm",       "LD (DE), A",       "INC DE",
    "INC D",            "DEC D",            "LD D, imm",        "RLA",
    "JR imm",           "ADD HL, DE",       "LD A, (DE)",       "DEC DE",
    "INC E",            "DEC E",            "LD E, imm",        "RRA",
    // 0x2
    "JP NZ, imm",       "LD HL, imm",       "LDI (HL), A",      "INC HL",
    "INC H",            "DEC H",            "LD H, imm",        "DAA",
    "JP Z, imm",        "ADD HL, HL",       "LDI A, (HL)",      "DEC HL",
    "INC L",            "DEC L",            "LD L, imm",        "CPL",
    // 0x3
    "JP NC, imm",       "LD SP, imm",       "LDD (HL), A",      "INC SP",
    "INC (HL)",         "DEC (HL)",         "LD (HL), imm",     "SCF",
    "JP C, imm",        "ADD HL, SP",       "LDD A, (HL)",      "DEC SP",
    "INC A",            "DEC A",            "LD A, imm",        "CCF",
    // 0x4
    "LD B, B",          "LD B, C",          "LD B, D",          "LD B, E",
    "LD B, H",          "LD B, L",          "LD B, (HL)",       "LD B, A",
    "LD C, B",          "LD C, C",          "LD C, D",          "LD C, E",
    "LD C, H",          "LD C, L",          "LD C, (HL)",       "LD C, A",
    // 0x5
    "LD D, B",          "LD D, C",          "LD D, D",          "LD D, E",
    "LD D, H",          "LD D, L",          "LD D, (HL)",       "LD D, A",
    "LD E, B",          "LD E, C",          "LD E, D",          "LD E, E",
    "LD E, H",          "LD E, L",          "LD E, (HL)",       "LD E, A",
    // 0x6
    "LD H, B",          "LD H, C",          "LD H, D",          "LD H, E",
    "LD H, H",          "LD H, L",          "LD H, (HL)",       "LD H, A",
    "LD L, B",          "LD L, C",          "LD L, D",          "LD L, E",
    "LD L, H",          "LD L, L",          "LD L, (HL)",       "LD L, A",
    // 0x7
    "LD (HL), B",       "LD (HL), C",       "LD (HL), D",       "LD (HL), E",
    "LD (HL), H",       "LD (HL), L",       "HALT",             "LD (HL), A",
    "LD A, B",          "LD A, C",          "LD A, D",          "LD A, E",
    "LD A, H",          "LD A, L",          "LD A, (HL)",       "LD A, A",
    // 0x8
    "ADD A, B",         "ADD A, C",         "ADD A, D",         "ADD A, E",
    "ADD A, H",         "ADD A, L",         "ADD A, (HL)",      "ADD A, A",
    "ADC A, B",         "ADC A, C",         "ADC A, D",         "ADC A, E",
    "ADC A, H",         "ADC A, L",         "ADC A, (HL)",      "ADC A, A",
    // 0x9
    "SUB B",            "SUB C",            "SUB D",            "SUB E",
    "SUB H",            "SUB L",            "SUB (HL)",         "SUB A",
    "SBC A, B",         "SBC A, C",         "SBC A, D",         "SBC A, E",
    "SBC A, H",         "SBC A, L",         "SBC A, (HL)",      "SBC A, A",
    // 0xA
    "AND B",            "AND C",            "AND D",            "AND E",
    "AND H",            "AND L",            "AND (HL)",         "AND A",
    "XOR B",            "XOR C",            "XOR D",            "XOR E",
    "XOR H",            "XOR L",            "XOR (HL)",         "XOR A",
    // 0xB
    "OR B",             "OR C",             "OR D",             "OR E",
    "OR H",             "OR L",             "OR (HL)",          "OR A",
    "CP B",             "CP C",             "CP D",             "CP E",
    "CP H",             "CP L",             "CP (HL)",          "CP A",
    // 0xC
    "RET NZ",           "POP BC",           "JP NZ, imm",       "JP imm",
    "CALL NZ, imm",     "PUSH BC",          "ADD A, imm",       "RST 0x0000",
    "RET Z",            "RET",              "JP Z, imm",        "CB INST",
    "CALL Z, imm",      "CALL imm",         "ADC A, imm",       "RST 0x0008",
    // 0xD
    "RET NC",           "POP DE",           "JP NC, imm",       "illegal",
    "CALL NC, imm",     "PUSH DE",          "SUB imm",          "RST 0x0010",
    "RET C",            "RETI",             "JP C, imm",        "illegal",
    "CALL C, imm",      "illegal",          "SBC A, imm",       "RST 0x0018",
    // 0xE
    "LD (0xFF00+imm), A","POP HL",          "LD (C), A",        "illegal",
    "illegal",          "PUSH HL",          "AND imm",          "RST 0x0020",
    "ADD SP, imm",      "JP (HL)",          "LD (imm), A",      "illegal",
    "illegal",          "illegal",          "XOR imm",          "RST 0x0028",
    // 0xF
    "LD A, (0xFF00+imm)","POP AF",          "LD A, (C)",        "DI",
    "illegal",          "PUSH AF",          "OR imm",           "RST 0x0030",
    "LD HL, SP+/-imm",  "LD SP, HL",        "LD A, (imm)",      "EI",
    "illegal",          "illegal",          "CP imm",           "RST 0x0038"
    };
*/



// Interrupts
VBLANK_INTR     = 0x01;
LCDC_INTR       = 0x02;
TIMER_INTR      = 0x04;
SERIAL_INTR     = 0x08;
CONTROL_INTR    = 0x10;
ANY_INTR        = 0x1F;

// Interrupt jump addresses
VBLANK_INTR_ADDR    = 0x0040;
LCDC_INTR_ADDR      = 0x0048;
TIMER_INTR_ADDR     = 0x0050;
SERIAL_INTR_ADDR    = 0x0058;
CONTROL_INTR_ADDR   = 0x0060;

// memory section sizes
// DMG
//#define WRAM_SIZE       0x2000
//#define VRAM_SIZE       0x2000
// CGB
WRAM_SIZE       = 0x8000;
VRAM_SIZE       = 0x4000;
HRAM_SIZE       = 0x0100;
OAM_SIZE        = 0x00A0;

// flags: ZERO, NEGATIVE OP (1: SUB, 0: ADD), HALF CARRY, CARRY
ZF_FLAG         = 0x0080;
N_FLAG          = 0x0040;
H_FLAG          = 0x0020;
CY_FLAG         = 0x0010;

// cart section sizes
ROM_BANK_SIZE   = 0x4000;
WRAM_BANK_SIZE  = 0x1000;
CRAM_BANK_SIZE  = 0x2000;
VRAM_BANK_SIZE  = 0x2000;

// CGB support
//CGB_OPTIONAL    = 0x80;
//CGB_EXCLUSIVE   = 0xC0;
CGB_OPTIONAL    = 0x100;
CGB_EXCLUSIVE   = 0x100;

// ROM info locations
ROM_CGB_SUPPORT = 0x0143;
ROM_MBC_INFO    = 0x0147;
ROM_BANK_COUNT  = 0x0148;
ROM_RAM_SIZE    = 0x0149;

// address book
ROM_0_ADDR      = 0x0000;
ROM_N_ADDR      = 0x4000;
VRAM_ADDR       = 0x8000;
CART_RAM_ADDR   = 0xA000;
WRAM_0_ADDR     = 0xC000;
WRAM_1_ADDR     = 0xD000;
ECHO_ADDR       = 0xE000;
OAM_ADDR        = 0xFE00;
UNUSED_ADDR     = 0xFEA0;
IO_ADDR         = 0xFF00;
HRAM_ADDR       = 0xFF00;
INTR_EN_ADDR    = 0xFFFF;

// MBC control
RAM_ENABLE_ADDR = 0x0000;
ROM_BANK_ADDR   = 0x2000;
RAM_BANK_ADDR   = 0x4000;
RAM_MODE_ADDR   = 0x6000;

// DIV // 16328 Hz
DIV_CYCLES      = 256;

// TAC
TAC_ENABLE          = 0x04;
TAC_RATE            = 0x03;
TAC_CYCLES 			= [1024, 16, 64, 256];

// LCD
LCD_LINE_CYCLES     = 456;
LCD_MODE_0_CYCLES   = 0;
LCD_MODE_2_CYCLES   = 204;
LCD_MODE_3_CYCLES   = 284;
LCD_VERT_LINES      = 154;
LCD_WIDTH           = 160;
LCD_HEIGHT          = 144;

// LCD modes
LCD_HBLANK          = 0x0;
LCD_VBLANK          = 0x1;
LCD_SEARCH_OAM      = 0x2;
LCD_TRANSFER        = 0x3;

// STAT register controls
STAT_LYC_INTR       = 0x40;
STAT_MODE_2_INTR    = 0x20;
STAT_MODE_1_INTR    = 0x10;
STAT_MODE_0_INTR    = 0x08;
STAT_LYC_COINC      = 0x04;
STAT_MODE           = 0x03;
STAT_USER_BITS      = 0xF8;
 
// LCDC controls
LCDC_ENABLE         = 0x80;
LCDC_WINDOW_MAP     = 0x40;
LCDC_WINDOW_ENABLE  = 0x20;
LCDC_TILE_SELECT    = 0x10;
LCDC_BG_MAP         = 0x08;
LCDC_OBJ_SIZE       = 0x04;
LCDC_OBJ_ENABLE     = 0x02;
LCDC_BG_ENABLE      = 0x01;

// SPRITE controls
NUM_SPRITES         = 0x28;
MAX_SPRITES_LINE    = 0x0A;
OBJ_PRIORITY        = 0x80;
OBJ_FLIP_Y          = 0x40;
OBJ_FLIP_X          = 0x20;
OBJ_PALETTE         = 0x10;
// CGB SPRITE controls
OBJ_VRAM_BANK       = 0x08;
OBJ_CPALETTE        = 0x07;

// CGB BG Map Attributes
BMAP_PRIORITY       = 0x80;
BMAP_FLIP_Y         = 0x40;
BMAP_FLIP_X         = 0x20;
BMAP_VRAM_BANK      = 0x08;
BMAP_CPALETTE       = 0x07;

// INPUT
INPUT_P1            = 0xFF00;
INPUT_KEYS          = 0x0F;
INPUT_P14           = 0x10;
INPUT_P15           = 0x20;

// VRAM Locations
VRAM_TILES_1        = (0x8000 - VRAM_ADDR);
VRAM_TILES_2        = (0x8800 - VRAM_ADDR);
VRAM_BMAP_1         = (0x9800 - VRAM_ADDR);
VRAM_BMAP_2         = (0x9C00 - VRAM_ADDR);
VRAM_TILES_3        = (0x8000 - VRAM_ADDR + VRAM_BANK_SIZE);
VRAM_TILES_4        = (0x8800 - VRAM_ADDR + VRAM_BANK_SIZE);

// CGB DMA
HDMA_OFF            = 0x80;
HDMA_HBLANK         = 0x80;
HDMA_LENGTH         = 0x7F;

// CGB PALETTE ACCESS
PAL_AUTO_INCREMENT  = 0x80;
PAL_INDEX           = 0x3F;
PAL_CONTROL_BITS    = (PAL_AUTO_INCREMENT | PAL_INDEX);

// cart MBC / memory info
CART_MBC = [
    0, 1, 1, 1,-1, 2, 2,-1, 0, 0,-1, 0, 0, 0,-1, 3,
    3, 3, 3, 3,-1,-1,-1,-1,-1, 5, 5, 5, 5, 5, 5, 0
    ];

CART_RAM = [
    0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0,
    1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0
    ];

NUM_ROM_BANKS = [
    2, 4, 8,16,32,64,128, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0,72,80,96, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    ];

NUM_RAM_BANKS = [0, 1, 1, 4, 16];

// Nintendo (R)
DMG_BIOS = [ 
    0x31, 0xFE, 0xFF, 0xAF, 0x21, 0xFF, 0x9F, 0x32, 0xCB, 0x7C, 0x20, 0xFB, 0x21, 0x26, 0xFF, 0x0E,
    0x11, 0x3E, 0x80, 0x32, 0xE2, 0x0C, 0x3E, 0xF3, 0xE2, 0x32, 0x3E, 0x77, 0x77, 0x3E, 0xFC, 0xE0,
    0x47, 0x11, 0x04, 0x01, 0x21, 0x10, 0x80, 0x1A, 0xCD, 0x95, 0x00, 0xCD, 0x96, 0x00, 0x13, 0x7B,
    0xFE, 0x34, 0x20, 0xF3, 0x11, 0xD8, 0x00, 0x06, 0x08, 0x1A, 0x13, 0x22, 0x23, 0x05, 0x20, 0xF9, 
    0x3E, 0x19, 0xEA, 0x10, 0x99, 0x21, 0x2F, 0x99, 0x0E, 0x0C, 0x3D, 0x28, 0x08, 0x32, 0x0D, 0x20,
    0xF9, 0x2E, 0x0F, 0x18, 0xF3, 0x67, 0x3E, 0x64, 0x57, 0xE0, 0x42, 0x3E, 0x91, 0xE0, 0x40, 0x04, 
    0x1E, 0x02, 0x0E, 0x0C, 0xF0, 0x44, 0xFE, 0x90, 0x20, 0xFA, 0x0D, 0x20, 0xF7, 0x1D, 0x20, 0xF2, 
    0x0E, 0x13, 0x24, 0x7C, 0x1E, 0x83, 0xFE, 0x62, 0x28, 0x06, 0x1E, 0xC1, 0xFE, 0x64, 0x20, 0x06, 
    0x7B, 0xE2, 0x0C, 0x3E, 0x87, 0xE2, 0xF0, 0x42, 0x90, 0xE0, 0x42, 0x15, 0x20, 0xD2, 0x05, 0x20, 
    0x4F, 0x16, 0x20, 0x18, 0xCB, 0x4F, 0x06, 0x04, 0xC5, 0xCB, 0x11, 0x17, 0xC1, 0xCB, 0x11, 0x17, 
    0x05, 0x20, 0xF5, 0x22, 0x23, 0x22, 0x23, 0xC9, 0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 
    0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D, 0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E, 
    0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99, 0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC, 
    0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E, 0x3C, 0x42, 0xB9, 0xA5, 0xB9, 0xA5, 0x42, 0x3C, 
    0x21, 0x04, 0x01, 0x11, 0xA8, 0x00, 0x1A, 0x13, 0xBE, 0x20, 0xFE, 0x23, 0x7D, 0xFE, 0x34, 0x20, 
    0xF5, 0x06, 0x19, 0x78, 0x86, 0x23, 0x05, 0x20, 0xFB, 0x86, 0x20, 0xFE, 0x3E, 0x01, 0xE0, 0x50
    ];


// CPU //////////////////////////////
OP_CYCLES = [
//   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
     4,12, 8, 8, 4, 4, 8, 4,20, 8, 8, 8, 4, 4, 8, 4,    // 0x00
     4,12, 8, 8, 4, 4, 8, 4, 8, 8, 8, 8, 4, 4, 8, 4,    // 0x10
     8,12, 8, 8, 4, 4, 8, 4, 8, 8, 8, 8, 4, 4, 8, 4,    // 0x20
     8,12, 8, 8,12,12,12, 4, 8, 8, 8, 8, 4, 4, 8, 4,    // 0x30
     4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,    // 0x40
     4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,    // 0x50
     4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,    // 0x60
     8, 8, 8, 8, 8, 8, 4, 8, 4, 4, 4, 4, 4, 4, 8, 4,    // 0x70
     4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,    // 0x80
     4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,    // 0x90
     4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,    // 0xA0
     4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,    // 0xB0
     8,12,12,12,12,16, 8,32, 8, 8,12, 8,12,12, 8,32,    // 0xC0
     8,12,12, 0,12,16, 8,32, 8, 8,12, 0,12, 0, 8,32,    // 0xD0
    12,12, 8, 0, 0,16, 8,32,16, 4,16, 0, 0, 0, 8,32,    // 0xE0
    12,12, 8, 4, 0,16, 8,32,12, 8,16, 4, 0, 0, 8,32     // 0xF0
];


// gameboy registers / memory / data
var ROM;
var CRAM;
var ROM_SIZE;
var CRAM_SIZE;

var gb_bios_enable = 1;
var gb_halt = 0;
var gb_ime = 1;
var gb_keys = 0;
var gb_mbc = 0;
var gb_cram = 0;
var gb_frame = 0;
var gb_rtc = [];
for (var i = 0; i < 5; i++) gb_rtc[i] = 0;
var lcd_mode = 0;

// CGB
var cgb_enable = 0;
var cgb_double = 0;

// CPU
var OP;
var inst_cycles;

// Memory
var WRAM = [];
var VRAM = [];
var HRAM = [];
var OAM = [];

for (var i = 0; i < WRAM_SIZE; i++)
	WRAM[i] = 0x00;
for (var i = 0; i < VRAM_SIZE; i++)
	VRAM[i] = 0x00;
for (var i = 0; i < HRAM_SIZE; i++)
	HRAM[i] = 0x00;
for (var i = 0; i < OAM_SIZE; i++)
	OAM[i] = 0x00;	


// TIMERS
var cpu_count;
var lcd_count;
var div_count;
var tima_count;
var tac_enable;
var tac_rate;

// DMG PALETTES
var BGP = [];
var OBJP = [];

for (var i = 0; i < 4; i++)
	BGP[i] = 0;
for (var i = 0; i < 8; i++)
	OBJP[i] = 0;

// CGB PALETTES
var BCPD = [];
var OCPD = [];

for (var i = 0; i < 32*2; i++)
	{
	BCPD[i] = 0;
	OCPD[i] = 0;
	}

// REGISTERS
var R_P1,    R_SB,    R_SC,    R_DIV;
var R_TIMA,  R_TMA,   R_TAC,   R_IF;
var R_NR10,  R_NR11,  R_NR12,  R_NR14;
var R_NR21,  R_NR22,  R_NR24,  R_NR30;
var R_NR31,  R_NR32,  R_NR33,  R_NR41;
var R_NR42,  R_NR43,  R_NR44,  R_NR50;
var R_NR51,  R_NR52,  R_LCDC,  R_STAT;
var R_SCY,   R_SCX,   R_LY,    R_LYC;
var R_DMA,   R_BGP,   R_OBP0,  R_OBP1;
var R_WY,    R_WX,    R_IE;

// CGB REGISTERS
var R_HDMA,  R_BCPS,  R_OCPS,  R_KEY1;
var R_HDMAS;
var R_HDMAD;

// CPU REGISTERS
var R_A;
var R_B, R_C;
var R_D, R_E;
var R_H, R_L;
var SP,  PC;

// FLAG REGISTER
var F_Z, F_N, F_H, F_C;

// memory banks n stuff
var rom_bank;
var rom_banks;
var wram_bank;
var vram_bank;
var cram_bank;
var cram_banks;
var cram_enable;
var cram_mode;

// canvas
var screen;

// functions

function LoadROM(rom, size, save, save_size)
    {
    ROM         = rom;
    ROM_SIZE    = size;
    CRAM        = save;
    CRAM_SIZE   = save_size;
    gb_mbc      = CART_MBC[ROM[ROM_MBC_INFO]];
    gb_cram     = CART_RAM[ROM[ROM_MBC_INFO]];
    rom_banks   = NUM_ROM_BANKS[ROM[ROM_BANK_COUNT]];
    cram_banks  = NUM_RAM_BANKS[ROM[ROM_RAM_SIZE]];
    cgb_enable  = (ROM[ROM_CGB_SUPPORT] == CGB_EXCLUSIVE ||
                    ROM[ROM_CGB_SUPPORT] == CGB_OPTIONAL) ? 1 : 0;

    PowerUp();
    }

function GetSaveSize(rom)
    {
    ram_sizes = [0x00, 0x800, 0x2000, 0x8000, 0x20000];
    return ram_sizes[rom[ROM_RAM_SIZE]];
    }

function PowerUp()
    {
    // GB
    gb_halt = 0;
    gb_ime = 1;
    gb_bios_enable = cgb_enable ? 0 : 0;
    lcd_mode = 0;

    // MBC
    rom_bank    = 0x0001;
    wram_bank   = 0x01;
    vram_bank   = 0x00;
    cram_bank   = 0x00;
    cram_enable = 0;
    cram_mode   = 0;
    
    // CPU registers
    R_A = (cgb_enable ? 0x11 : 0x01);
    F_Z = 0x01;
    F_N = 0x00;
    F_H = 0x01;
    F_C = 0x01;
    R_B = 0x00;
    R_C = 0x13;
    R_D = 0x00;
    R_E = 0xD8;
    R_H = 0x01;
    R_L = 0x4D;
    SP  = 0xFFFE;

    if (gb_bios_enable)
        PC  = 0x0000;
    else
        PC  = 0x0100;
    
    // timer
    cpu_count   = 0x0000;
    lcd_count   = 0x0000;
    div_count   = 0x0000;
    tima_count  = 0x0000;
    tac_enable  = 0x00;
    tac_rate    = 0x00;
    
    // more registers
	R_P1		= 0x00;
	R_DIV       = 0x00;
    R_TIMA      = 0x00;
    R_TMA       = 0x00;
    R_TAC       = 0x00;
	R_IF		= 0x00;	
    R_NR10      = 0x80;
    R_NR11      = 0xBF;
    R_NR12      = 0xF3;
    R_NR14      = 0xBF;
    R_NR21      = 0x3F;
    R_NR22      = 0x00;
    R_NR24      = 0xBF;
    R_NR30      = 0x7F;
    R_NR31      = 0xFF;
    R_NR32      = 0x9F;
    R_NR33      = 0xBF;
    R_NR41      = 0xFF;
    R_NR42      = 0x00;
    R_NR43      = 0x00;
    R_NR44      = 0xBF;
    R_NR50      = 0x77;
    R_NR51      = 0xF3;
    R_NR52      = 0xF1;
    R_LCDC      = 0x91;
    R_SCY       = 0x00;
    R_SCX       = 0x00;
	R_LY		= 0x00;
    R_LYC       = 0x00;
	R_DMA		= 0x00;
    WRITE(0xFF47, 0xFC);    // BGP
    WRITE(0xFF48, 0xFF);    // OBJP0
    WRITE(0xFF49, 0x0F);    // OBJP1
    R_WY        = 0x00;
    R_WX        = 0x00;
    R_IE        = 0x00;

    // CGB registers
    R_HDMA      = 0xFF;
    R_BCPS      = 0x00;
    R_OCPS      = 0x00;
    R_KEY1      = 0x00;
    R_HDMAS     = 0x0000;
    R_HDMAD     = 0x8000;

    // CGB palette
    for (var i = 0; i < 32*2; i++)
        BCPD[i] = 0xFF;
    }

function run()
{
	// passed: 2, 4, 5, 6, 7, 8, 9, 10 failed: :3, 11
	LoadROM(KirbyROM, KirbyROM.length, [], 0);
    //LoadROM(Kirby2ROM, Kirby2ROM.length, [], 0);
	screen = document.getElementById("game").getContext("2d");
	document.onkeydown = KeyPress;
	document.onkeyup = KeyRelease;
	setTimeout(DoFrame, 0);
}

// controls
var gb_keys = 0x00;
var KEY_MAPPING = {39: 0, 37: 1, 38: 2, 40: 3, 90: 4, 88: 5, 16: 6, 13: 7};
function UpdateP1()
    {
    R_P1 |= 0x0F;
    if (!(R_P1 & INPUT_P14))
        R_P1 &= 0xF0 | ((gb_keys & 0x0F) ^ 0x0F);
    if (!(R_P1 & INPUT_P15))
        R_P1 &= 0xF0 | (((gb_keys >> 4) & 0x0F) ^ 0x0F);
    }

function KeyPress(evt)
    {
	var key;
	if (!evt) evt = window.event;
	if (KEY_MAPPING[evt.keyCode] !== undefined)
		key = KEY_MAPPING[evt.keyCode];
	else
		return;
	
    gb_keys |= 0x01 << key;
    UpdateP1();
    //R_IF |= CONTROL_INTR;
    }

function KeyRelease(evt)
    {
	var key;
	if (!evt) evt = window.event;
	if (KEY_MAPPING[evt.keyCode] !== undefined)
		key = KEY_MAPPING[evt.keyCode];
	else
		return;
	
    gb_keys &= (0x01 << key) ^ 0xFF;
    UpdateP1();
    //R_IF |= CONTROL_INTR;
    }

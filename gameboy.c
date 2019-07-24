/**********
 gameboy.c
 --------------------
 GameBoy emulator
 Credits: Greg Tourville
*/

#include "gameboy.h"

// CART /////////////////////////////
u8* ROM;
u8* CRAM;
u32 ROM_SIZE;
u32 CRAM_SIZE;

// memory banks and other things
u16 rom_bank;
u16 rom_banks;
u8  wram_bank;
u8  vram_bank;
u8  cram_bank;
u8  cram_banks;
u8  cram_enable;
u8  cram_mode;

// cart mbc / memory info
u8 CART_MBC[] = 
    {
    0, 1, 1, 1,-1, 2, 2,-1, 0, 0,-1, 0, 0, 0,-1, 3,
    3, 3, 3, 3,-1,-1,-1,-1,-1, 5, 5, 5, 5, 5, 5, 0
    };

u8 CART_RAM[] = 
    {
    0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0,
    1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0
    };

u16 NUM_ROM_BANKS[] =
    {
    2, 4, 8,16,32,64,128, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0,72,80,96, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };

u8  NUM_RAM_BANKS[] = {0, 1, 1, 4, 16};

// GB ///////////////////////////////
u8 gb_bios_enable = 1;
u8 opt_use_gb_bios = 1;
u8 gb_halt = 0;
u8 gb_ime = 1;
u8 gb_keys = 0;
u8 gb_mbc = 0;
u8 gb_cram = 0;
u8 gb_frame = 0;
u8 gb_rtc[5];
u8 lcd_mode = 0;

// CGB
u8 cgb_enable = 0;
u8 cgb_double = 0;

// CPU
u8 OP;
u8 inst_cycles;

// screen
u8  gb_fb[LCD_HEIGHT][LCD_WIDTH];
u16 cgb_fb[LCD_HEIGHT][LCD_WIDTH];

// char debug_msg[0x100];

// MEMORY ///////////////////////////
u8 WRAM[WRAM_SIZE];
u8 VRAM[VRAM_SIZE];
u8 HRAM[HRAM_SIZE];
u8 OAM[OAM_SIZE];

// TIMERS
u32 cpu_count;
u32 lcd_count;
u32 audio_count;
u32 div_count;
u32 tima_count;
u8  tac_enable;
u8  tac_rate;

const u32 TAC_CYCLES[4] = {1024, 16, 64, 256};

// PALETTES
u8 BGP[4];
u8 OBJP[8];

// CGB PALETTES
u16 BCPD[4*8];
u16 OCPD[4*8];

// 8-bit convenience
u8* R_BCPD = (u8*)BCPD;
u8* R_OCPD = (u8*)OCPD;

// REGISTERS
u8 R_P1;    u8 R_SB;    u8 R_SC;    u8 R_DIV;
u8 R_TIMA;  u8 R_TMA;   u8 R_TAC;   u8 R_IF;
u8 R_LCDC;  u8 R_STAT;
u8 R_SCY;   u8 R_SCX;   u8 R_LY;    u8 R_LYC;
u8 R_DMA;   u8 R_BGP;   u8 R_OBP0;  u8 R_OBP1;
u8 R_WY;    u8 R_WX;    u8 R_IE;

// SOUND
u8 R_NR10;  u8 R_NR11;  u8 R_NR12;  u8 R_NR13;  u8 R_NR14;  
u8 R_NR20;  u8 R_NR21;  u8 R_NR22;  u8 R_NR23;  u8 R_NR24;  
u8 R_NR30;  u8 R_NR31;  u8 R_NR32;  u8 R_NR33;  u8 R_NR34;  
u8 R_NR40;  u8 R_NR41;  u8 R_NR42;  u8 R_NR43;  u8 R_NR44;  
u8 R_NR50;  u8 R_NR51;  u8 R_NR52;

// CGB REGISTERS
u8 R_HDMA;  u8 R_BCPS;  u8 R_OCPS;  u8 R_KEY1;
u16 R_HDMAS;
u16 R_HDMAD;

// Read
u8 READ(u16 addr)
    {
    switch ((addr >> 12) & 0xF)
        {
        case 0x0:
#ifdef DMG_BIOS_ENABLE
            if (gb_bios_enable && addr < 0x100)
                {
                return DMG_BIOS[addr];
                }
#endif
        case 0x1:
        case 0x2:
        case 0x3:
            return ROM[addr];
            
        case 0x4:
        case 0x5:
        case 0x6:
        case 0x7:
            if (gb_mbc == 1 && cram_mode)
                return ROM[addr + ((rom_bank & 0x1F) - 1)*ROM_BANK_SIZE];
            else
                return ROM[addr + (rom_bank-1)*ROM_BANK_SIZE];
            
        case 0x8:
        case 0x9:
            return VRAM[addr - VRAM_ADDR];
            //return VRAM[addr - VRAM_ADDR + vram_bank*VRAM_BANK_SIZE];
            
        case 0xA:
        case 0xB:
            if (gb_cram && cram_enable)
                {
                if (gb_mbc == 3 && cram_bank >= 0x08)
                    return gb_rtc[cram_bank - 0x08];
                else if ((cram_mode || gb_mbc != 1) && cram_bank < cram_banks)
                    return CRAM[addr - CART_RAM_ADDR + cram_bank*CRAM_BANK_SIZE];
                else
                    return CRAM[addr - CART_RAM_ADDR];
                }
            return 0;
        
        case 0xC:
            return WRAM[addr - WRAM_0_ADDR];
        
        case 0xD:
            return WRAM[addr - WRAM_1_ADDR + wram_bank*WRAM_BANK_SIZE];
        
        case 0xE:
            return WRAM[addr - ECHO_ADDR];
        
        case 0xF:
            if (addr < OAM_ADDR)
                return WRAM[addr - ECHO_ADDR + (wram_bank-1)*WRAM_BANK_SIZE];
            if (addr < UNUSED_ADDR)
                return OAM[addr - OAM_ADDR];
            if (addr < IO_ADDR)
                return 0; // unused
            switch (addr & 0xFF)        // 0xFF00 .. 0xFFFF
                {
                // I/O Registers
                case 0x00:
                    return 0xC0 | R_P1;
                case 0x01: return R_SB;
                case 0x02: return R_SC;
                    
                // Timer Registers                    
                case 0x04: return R_DIV;
                case 0x05: return R_TIMA;
                case 0x06: return R_TMA;
                case 0x07: return R_TAC;
                    
                // Interrupt Flag Register
                case 0x0F: return R_IF;
                    
                // Sound Registers
                case 0x10: case 0x11: case 0x12: case 0x13: 
                case 0x14: case 0x15: case 0x16: case 0x17: 
                case 0x18: case 0x19: case 0x1A: case 0x1B: 
                case 0x1C: case 0x1D: case 0x1E: case 0x1F: 
                case 0x20: case 0x21: case 0x22: case 0x23: 
                case 0x24: case 0x25: case 0x26:/*case 0x27:
                case 0x28: case 0x29: case 0x2A: case 0x2B: 
                case 0x2C: case 0x2D: case 0x2E: case 0x2F:*/
                case 0x30: case 0x31: case 0x32: case 0x33: 
                case 0x34: case 0x35: case 0x36: case 0x37: 
                case 0x38: case 0x39: case 0x3A: case 0x3B: 
                case 0x3C: case 0x3D: case 0x3E: case 0x3F: 
                    return AUDIO_READ(addr & 0xFF);
                
                // LCD Registers
                case 0x40: return R_LCDC;
                case 0x41: 
                    return (R_STAT & STAT_USER_BITS) | 
                        (R_LCDC & LCDC_ENABLE ? lcd_mode : LCD_VBLANK);
                case 0x42: return R_SCY;
                case 0x43: return R_SCX;
                case 0x44: return R_LY;
                case 0x45: return R_LYC;

                // CGB speed setting
                case 0x4D: return (cgb_double << 7) | (R_KEY1 & 0x01);

                // CGB vram bank
                case 0x4F: return vram_bank;
                    
                // DMA Register
                case 0x46: return R_DMA;
                
                // Drawing Registers
                case 0x47: return R_BGP;
                case 0x48: return R_OBP0;
                case 0x49: return R_OBP1;
                case 0x4A: return R_WY;
                case 0x4B: return R_WX;

                // CGB HDMA
                case 0x51: return (R_HDMAS >> 8) & 0xFF;
                case 0x52: return (R_HDMAS & 0xFF);
                case 0x53: return (R_HDMAD >> 8) & 0xFF;
                case 0x54: return (R_HDMAD & 0xFF);
                case 0x55: return R_HDMA;

                // CGB Palette Registers
                case 0x68: return R_BCPS;
                case 0x69: return R_BCPD[R_BCPS & PAL_INDEX];
                case 0x6A: return R_OCPS;
                case 0x6B: return R_OCPD[R_OCPS & PAL_INDEX];

                // CGB wram bank
                case 0x70: return wram_bank;
                    
                // Interrupt Enable Register
                case 0xFF: return R_IE;
                
                // High RAM
                default:   return HRAM[addr - HRAM_ADDR];
                }
        }
    return 0;
    }

// Write
void WRITE(u16 addr, u8 val)
    {
    switch (addr >> 12)
        {
        case 0x0:
        case 0x1:
            if (gb_mbc == 2 && addr & 0x10)
                return;
            else if (gb_mbc > 0 && gb_cram)
                cram_enable = ((val & 0x0F) == 0x0A);
            return;
            
        case 0x2:
            if (gb_mbc == 5)
                {
                rom_bank = (rom_bank & 0x100) | val;
                rom_bank = rom_bank % rom_banks;
                return;
                }
        case 0x3:
            if (gb_mbc == 1)
                {
                //rom_bank = val & 0x7;
                rom_bank = (val & 0x1F) | (rom_bank & 0x60);
                if ((rom_bank & 0x1F) == 0x00)
                    rom_bank++;
                }
            else if (gb_mbc == 2 && addr & 0x10)
                {
                rom_bank = val & 0x0F;
                if (!rom_bank) 
                    rom_bank++;
                }
            else if (gb_mbc == 3)
                {
                rom_bank = val & 0x7F;
                if (!rom_bank) 
                    rom_bank++;
                }
            else if (gb_mbc == 5)
                rom_bank = (val & 0x01) << 8 | (rom_bank & 0xFF);
            rom_bank = rom_bank % rom_banks;
            return;
            
        case 0x4:
        case 0x5:
            if (gb_mbc == 1)
                {
                cram_bank = (val & 3);
                rom_bank = ((val & 3) << 5) | (rom_bank & 0x1F);
                rom_bank = rom_bank % rom_banks;
                }
            else if (gb_mbc == 3)
                cram_bank = val;
            else if (gb_mbc == 5)
                cram_bank = (val & 0x0F);
            return;
            
        case 0x6:
        case 0x7:
            //if (gb_mbc == 1 && gb_cram)
            cram_mode = (val & 1);
            return;
            
        case 0x8:
        case 0x9:
            VRAM[addr - VRAM_ADDR + vram_bank*VRAM_BANK_SIZE] = val;
            return;
            
        case 0xA:
        case 0xB:
            if (gb_cram && cram_enable)
                {
                if (gb_mbc == 3 && cram_bank >= 0x08)
                    gb_rtc[cram_bank - 0x08] = val;
                //else if ((cram_mode || gb_mbc != 1) && cram_bank < cram_banks)
                else if (cram_mode && cram_bank < cram_banks)
                    CRAM[addr - CART_RAM_ADDR + cram_bank*CRAM_BANK_SIZE] = val;
                else
                    CRAM[addr - CART_RAM_ADDR] = val;
                }
            return;
        
        case 0xC:
            WRAM[addr - WRAM_0_ADDR] = val;
            return;
        
        case 0xD:
            WRAM[addr - WRAM_1_ADDR + wram_bank*WRAM_BANK_SIZE] = val;
            return;
        
        case 0xE:
            WRAM[addr - ECHO_ADDR] = val;
            return;
        
        case 0xF:
            if (addr < OAM_ADDR)
                {
                WRAM[addr - ECHO_ADDR + (wram_bank-1)*WRAM_BANK_SIZE] = val;
                return;
                }
            if (addr < UNUSED_ADDR)
                {
                OAM[addr - OAM_ADDR] = val;
                return;
                }
            if (addr < IO_ADDR)
                return;    // unused
            switch (addr & 0xFF)        // 0xFF00 .. 0xFFFF
                {
                // I/O Registers
                case 0x00:      // Controls
                    R_P1 = val & 0x30;
                    UpdateP1();
                    return;
                case 0x01: R_SB = val;      return;
                case 0x02: R_SC = val;      return;
                    
                // Timer Registers                    
                case 0x04: R_DIV = 0x00;    return;
                case 0x05: R_TIMA = val;    return;
                case 0x06: R_TMA = val;     return;
                case 0x07:
                    R_TAC = val;
                    tac_enable = R_TAC & TAC_ENABLE;
                    tac_rate = R_TAC & TAC_RATE;
                    return;
                    
                // Interrupt Flag Register
                case 0x0F:
                    R_IF = val;
                    return;
                    
                // Sound Registers
                case 0x10: case 0x11: case 0x12: case 0x13: 
                case 0x14: case 0x15: case 0x16: case 0x17: 
                case 0x18: case 0x19: case 0x1A: case 0x1B: 
                case 0x1C: case 0x1D: case 0x1E: case 0x1F: 
                case 0x20: case 0x21: case 0x22: case 0x23: 
                case 0x24: case 0x25: case 0x26:/*case 0x27:
                case 0x28: case 0x29: case 0x2A: case 0x2B: 
                case 0x2C: case 0x2D: case 0x2E: case 0x2F:*/
                case 0x30: case 0x31: case 0x32: case 0x33: 
                case 0x34: case 0x35: case 0x36: case 0x37: 
                case 0x38: case 0x39: case 0x3A: case 0x3B: 
                case 0x3C: case 0x3D: case 0x3E: case 0x3F: 
                    AUDIO_WRITE(addr & 0xFF, val);
                    return;
                
                // LCD Registers
                case 0x40: R_LCDC = val;    return;
                case 0x41: R_STAT = val;    return;
                case 0x42: R_SCY = val;     return;
                case 0x43: R_SCX = val;     return;
                case 0x44: R_LY = val;      return;
                case 0x45: R_LYC = val;     return;
                    
                // DMA Register
                case 0x46: 
                    {
                    u8 offset;
                    R_DMA = (val % 0xF1);
                    for (offset = 0; offset < OAM_SIZE; offset++)
                        {
                        OAM[offset] = READ((R_DMA << 8) + offset);
                        }
                    return;
                    }
                // DMG Palette Registers
                case 0x47:
                    R_BGP = val;
                    BGP[0] = (R_BGP & 0x03);
                    BGP[1] = (R_BGP >> 2) & 0x03;
                    BGP[2] = (R_BGP >> 4) & 0x03;
                    BGP[3] = (R_BGP >> 6) & 0x03;
                    return;
                case 0x48:
                    R_OBP0 = val;
                    OBJP[0] = (R_OBP0 & 0x03);
                    OBJP[1] = (R_OBP0 >> 2) & 0x03;
                    OBJP[2] = (R_OBP0 >> 4) & 0x03;
                    OBJP[3] = (R_OBP0 >> 6) & 0x03;
                    return;
                case 0x49:
                    R_OBP1 = val;
                    OBJP[4] = (R_OBP1 & 0x03);
                    OBJP[5] = (R_OBP1 >> 2) & 0x03;
                    OBJP[6] = (R_OBP1 >> 4) & 0x03;
                    OBJP[7] = (R_OBP1 >> 6) & 0x03;
                    return;
                // Window position
                case 0x4A: R_WY = val;      return;
                case 0x4B: R_WX = val;      return;

                // CGB speed setting
                case 0x4D:
                    R_KEY1 = val & 0x1;
                    return;

                // CGB VRAM bank
                case 0x4F:
                    if (cgb_enable)
                        {
                        vram_bank = val & 0x1;
                        }
                    return;

                // turn off boot rom
                case 0x50: gb_bios_enable = 0; return;

                // CGB HDMA
                case 0x51:
                    if (cgb_enable && (R_HDMA & HDMA_OFF))
                        R_HDMAS = (val << 8) | (R_HDMAS & 0x00F0);
                    return;
                case 0x52:
                    if (cgb_enable && (R_HDMA & HDMA_OFF))
                        R_HDMAS = (R_HDMAS & 0xFF00) | (val & 0x00F0);
                    return;
                case 0x53:
                    if (cgb_enable && (R_HDMA & HDMA_OFF))
                        R_HDMAD = 0x8000 | ((val << 8) & 0x1F00) | (R_HDMAD & 0x00F0);
                    return;
                case 0x54:
                    if (cgb_enable && (R_HDMA & HDMA_OFF))
                        R_HDMAD = 0x8000 | (R_HDMAD & 0x1F00) | (val & 0x00F0);
                    return;  
                case 0x55:
                    if (val & HDMA_HBLANK)
                        {
                        // set length to copy
                        R_HDMA = val & HDMA_LENGTH;
                        }
                    else
                        {
                        if (R_HDMA & HDMA_OFF)
                            {
                            u16 i;
                            // blocking DMA
                            for (i = 0; i < (val + 1) * 0x10; i++)
                                WRITE(R_HDMAD+i, READ(R_HDMAS+i));
                            inst_cycles += 8 * (cgb_double + 1) * (val + 1);
                            R_HDMA = 0xFF;
                            }
                        else
                            {
                            // disable hblank dma
                            R_HDMA |= HDMA_OFF;
                            }
                        }
                    return;

                // CGB Palette Registers
                case 0x68: R_BCPS = val;    return;
                case 0x69:
                    R_BCPD[R_BCPS & PAL_INDEX] = val;
                    if (R_BCPS & PAL_AUTO_INCREMENT)
                        R_BCPS = PAL_CONTROL_BITS & (R_BCPS + 1);
                    return;
                case 0x6A: R_OCPS = val;    return;
                case 0x6B:
                    R_OCPD[R_OCPS & PAL_INDEX] = val;
                    if (R_OCPS & PAL_AUTO_INCREMENT)
                        R_OCPS = PAL_CONTROL_BITS & (R_OCPS + 1);
                    return;

                // CGB WRAM bank
                case 0x70:
                    if (cgb_enable)
                        {
                        wram_bank = val & 0x7;
                        if (wram_bank == 0x0)
                            wram_bank = 0x1;
                        }
                    return;
                    
                // Interrupt Enable Register
                case 0xFF: R_IE = val;      return;
                
                // High RAM
                default:
                    HRAM[addr - HRAM_ADDR] = val;
                    return;
                }
        }
    }


// CPU //////////////////////////////
u8 OP_CYCLES[0x100] = {
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
};

// CPU REGISTERS
u8 R_A;
u8 R_B; u8 R_C;
u8 R_D; u8 R_E;
u8 R_H; u8 R_L;
u16 SP;
u16 PC;

#define R_BC    (R_B << 8 | R_C)
#define R_DE    (R_D << 8 | R_E)
#define R_HL    (R_H << 8 | R_L)

// FLAG REGISTER
u8  F_Z;
u8  F_N;
u8  F_H;
u8  F_C;

// random variables
u8  N;  s8  SN;
u16 NN; u32 NNNN;
u8  D1, D2; // DAA

void RunFrame()
    {
    gb_frame = 0;
    while (!gb_frame)
        StepCPU();
    }

void StepCPU()
    {
    // handle interrupts
    if ((gb_ime || gb_halt) && (R_IF & R_IE & ANY_INTR))
        {
        // disable halt
        gb_halt = 0;
        
        if (gb_ime)
            {
            // disable interrupts
            gb_ime = 0;

            // PUSH PC
            WRITE(--SP, PC >> 8);
            WRITE(--SP, PC & 0xFF);
            
            // Call interrupt handler
            if (R_IF & R_IE & VBLANK_INTR)
                {
                PC = VBLANK_INTR_ADDR;
                R_IF ^= VBLANK_INTR;
                }
            else if (R_IF & R_IE & LCDC_INTR)
                {
                PC = LCDC_INTR_ADDR;
                R_IF ^= LCDC_INTR;
                }
            else if (R_IF & R_IE & TIMER_INTR)
                {
                PC = TIMER_INTR_ADDR;
                R_IF ^= TIMER_INTR;
                }
            else if (R_IF & R_IE & SERIAL_INTR)
                {
                PC = SERIAL_INTR_ADDR;
                R_IF ^= SERIAL_INTR;
                }
            else if (R_IF & R_IE & CONTROL_INTR)
                {
                PC = CONTROL_INTR_ADDR;
                R_IF ^= CONTROL_INTR;
                }
            }
        }
    
    // Execute one instruction
    OP = (gb_halt ? 0x00 : READ(PC++));
    inst_cycles = OP_CYCLES[OP];
    switch (OP)
        {
        case 0x00: // NOP
            break;
        case 0x01: // LD BC, imm
            R_C = READ(PC++);
            R_B = READ(PC++);
            break;
        case 0x02: // LD (BC), A
            WRITE(R_BC, R_A);
            break;
        case 0x03: // INC BC
            NN = R_BC + 1;
            R_B = NN >> 8;
            R_C = NN & 0xFF;
            break;
        case 0x04: // INC B
            R_B++;
            F_Z = (R_B == 0x00);
            F_N = 0;
            F_H = ((R_B & 0x0F) == 0x00);
            break;
        case 0x05: // DEC B
            R_B--;
            F_Z = (R_B == 0x00);
            F_N = 1;
            F_H = ((R_B & 0x0F) == 0x0F);
            break;
        case 0x06: // LD B, imm
            R_B = READ(PC++);
            break;
        case 0x07: // RLCA
            R_A = (R_A << 1) | (R_A >> 7);
            F_Z = 0;            
            F_N = 0;
            F_H = 0;
            F_C = (R_A & 0x01);
            break;
        case 0x08: // LD (imm), SP
            NN = READ(PC++);
            NN |= READ(PC++) << 8;
            WRITE(NN++, SP & 0xFF);
            WRITE(NN, SP >> 8);
            break;
        case 0x09: // ADD HL, BC
            NNNN = R_HL + R_BC;
            F_N = 0;
            F_H = (NNNN ^ R_HL ^ R_BC) & 0x1000 ? 1 : 0;
            F_C = (NNNN & 0xFFFF0000) ? 1 : 0;
            R_H = (NNNN & 0x0000FF00) >> 8;
            R_L = (NNNN & 0x000000FF);
            break;
        case 0x0A: // LD A, (BC)
            R_A = READ(R_BC);
            break;
        case 0x0B: // DEC BC
            NN = R_BC - 1;
            R_B = NN >> 8;
            R_C = NN & 0xFF;
            break;
        case 0x0C: // INC C
            R_C++;
            F_Z = (R_C == 0x00);
            F_N = 0;
            F_H = ((R_C & 0x0F) == 0x00);
            break;
        case 0x0D: // DEC C
            R_C--;
            F_Z = (R_C == 0x00);
            F_N = 1;
            F_H = ((R_C & 0x0F) == 0x0F);
            break;
        case 0x0E: // LD C, imm
            R_C = READ(PC++);
            break;
        case 0x0F: // RRCA
            F_C = R_A & 0x01;
            R_A = (R_A >> 1) | (R_A << 7);
            F_Z = 0;
            F_N = 0;
            F_H = 0;
            break;
        case 0x10: // STOP
            gb_halt = 1;
            if (cgb_enable)
                {
                // check for CGB speed change
                if (R_KEY1 & 0x01)
                    {
                    cgb_double = !cgb_double;
                    R_KEY1 = 0x00;
                    }
                }
            break;
        case 0x11: // LD DE, imm
            R_E = READ(PC++);
            R_D = READ(PC++);
            break;
        case 0x12: // LD (DE), A
            WRITE(R_DE, R_A);
            break;
        case 0x13: // INC DE
            NN = R_DE + 1;
            R_D = NN >> 8;
            R_E = NN & 0xFF;
            break;
        case 0x14: // INC D
            R_D++;
            F_Z = (R_D == 0x00);
            F_N = 0;
            F_H =  ((R_D & 0x0F) == 0x00);
            break;
        case 0x15: // DEC D
            R_D--;
            F_Z = (R_D == 0x00);
            F_N = 1;
            F_H = ((R_D & 0x0F) == 0x0F);
            break;
        case 0x16: // LD D, imm
            R_D = READ(PC++);
            break;
        case 0x17: // RLA
            N = R_A;
            R_A = R_A << 1 | F_C;
            F_Z = 0;            
            F_N = 0;
            F_H = 0;
            F_C = (N >> 7) & 0x01;
            break;
        case 0x18: // JR imm
            SN = (s8)READ(PC++);
            PC += SN;
            break;
        case 0x19: // ADD HL, DE
            NNNN = R_HL + R_DE;
            F_N = 0;
            F_H = (NNNN ^ R_HL ^ R_DE) & 0x1000 ? 1 : 0;
            F_C = (NNNN & 0xFFFF0000) ? 1 : 0;
            R_H = (NNNN & 0x0000FF00) >> 8;
            R_L = (NNNN & 0x000000FF);
            break;
        case 0x1A: // LD A, (DE)
            R_A = READ(R_DE);
            break;
        case 0x1B: // DEC DE
            NN = R_DE - 1;
            R_D = NN >> 8;
            R_E = NN & 0xFF;
            break;
        case 0x1C: // INC E
            R_E++;
            F_Z = (R_E == 0x00);
            F_N = 0;
            F_H = ((R_E & 0x0F) == 0x00);
            break;
        case 0x1D: // DEC E
            R_E--;
            F_Z = (R_E == 0x00);
            F_N = 1;
            F_H = ((R_E & 0x0F) == 0x0F);
            break;
        case 0x1E: // LD E, imm
            R_E = READ(PC++);
            break;
        case 0x1F: // RRA
            N = R_A;
            R_A = R_A >> 1 | (F_C << 7);
            F_Z = 0;
            F_N = 0;
            F_H = 0;
            F_C = N & 0x1;
            break;
        case 0x20: // JP NZ, imm
            SN = (s8)READ(PC++);
            if (!F_Z)
                {
                PC += SN;
                }
            break;
        case 0x21: // LD HL, imm
            R_L = READ(PC++);
            R_H = READ(PC++);
            break;
        case 0x22: // LDI (HL), A
            WRITE(R_HL, R_A);
            NN = R_HL + 1;
            R_H = NN >> 8;
            R_L = NN & 0xFF;
            break;
        case 0x23: // INC HL
            NN = R_HL + 1;
            R_H = NN >> 8;
            R_L = NN & 0xFF;
            break;
        case 0x24: // INC H
            R_H++;
            F_Z = (R_H == 0x00);
            F_N = 0;
            F_H = ((R_H & 0x0F) == 0x00);
            break;
        case 0x25: // DEC H
            R_H--;
            F_Z = (R_H == 0x00);
            F_N = 1;
            F_H = ((R_H & 0x0F) == 0x0F);
            break;
        case 0x26: // LD H, imm
            R_H = READ(PC++);
            break;
        case 0x27: // DAA
            D1 = R_A >> 4;
            D2 = R_A & 0x0F;
            if (F_N)
                {
                if (F_H) D2 -= 6;
                if (F_C) D1 -= 6;
                if (D2>9) D2 -= 6;
                if (D1 > 9)
                    {
                    D1 -= 6;
                    F_C = 1;
                    }
                }
            else
                {
                if (F_H) D2 += 6;
                if (F_C) D1 += 6;
                if (D2 > 9)
                    {
                    D2 -= 10;
                    D1++;
                    }
                if (D1 > 9)
                    {
                    D1 -= 10;
                    F_C = 1;
                    }
                }
            R_A = ((D1 << 4) & 0xF0) | (D2 & 0x0F);
            F_Z = (R_A == 0);
            F_H = 0;
            break;
        case 0x28: // JP Z, imm
            SN = (s8)READ(PC++);
            if (F_Z)
                {
                PC += SN;
                }
            break;
        case 0x29: // ADD HL, HL
            NNNN = R_HL + R_HL;
            F_N = 0;
            F_H = (NNNN & 0x1000) ? 1 : 0;
            F_C = (NNNN & 0xFFFF0000) ? 1 : 0;
            R_H = (NNNN & 0x0000FF00) >> 8;
            R_L = (NNNN & 0x000000FF);
            break;
        case 0x2A: // LDI A, (HL)
            R_A = READ(R_HL);
            NN = R_HL + 1;
            R_H = NN >> 8;
            R_L = NN & 0xFF;
            break;
        case 0x2B: // DEC HL
            NN = R_HL - 1;
            R_H = NN >> 8;
            R_L = NN & 0xFF;
            break;
        case 0x2C: // INC L
            R_L++;
            F_Z = (R_L == 0x00);
            F_N = 0;
            F_H = ((R_L & 0x0F) == 0x00);
            break;
        case 0x2D: // DEC L
            R_L--;
            F_Z = (R_L == 0x00);
            F_N = 1;
            F_H = ((R_L & 0x0F) == 0x0F);
            break;
        case 0x2E: // LD L, imm
            R_L = READ(PC++);
            break;
        case 0x2F: // CPL
            R_A = R_A ^ 0xFF;
            F_N = 1;
            F_H = 1;
            break;
        case 0x30: // JP NC, imm
            SN = (s8)READ(PC++);
            if (!F_C)
                {
                PC += SN;
                }
            break;
        case 0x31: // LD SP, imm
            SP = READ(PC++);
            SP |= READ(PC++) << 8;
            break;
        case 0x32: // LDD (HL), A
            WRITE(R_HL, R_A);
            NN = R_HL - 1;
            R_H = NN >> 8;
            R_L = NN & 0xFF;
            break;
        case 0x33: // INC SP
            SP++;
            break;
        case 0x34: // INC (HL)
            N = READ(R_HL) + 1;
            F_Z = (N == 0x00);
            F_N = 0;
            F_H = ((N & 0x0F) == 0x00);
            WRITE(R_HL, N);
            break;
        case 0x35: // DEC (HL)
            N = READ(R_HL) - 1;
            F_Z = (N == 0x00);
            F_N = 1;
            F_H = ((N & 0x0F) == 0x0F);
            WRITE(R_HL, N);
            break;
        case 0x36: // LD (HL), imm
            WRITE(R_HL, READ(PC++));
            break;
        case 0x37: // SCF
            F_N = 0;
            F_H = 0;
            F_C = 1;
            break;
        case 0x38: // JP C, imm
            SN = (s8)READ(PC++);
            if (F_C)
                {
                PC += SN;
                }
            break;
        case 0x39: // ADD HL, SP
            NNNN = R_HL + SP;
            F_N = 0;
            F_H = (NNNN ^ R_HL ^ SP) & 0x1000 ? 1 : 0;
            F_C = (NNNN & 0xFFFF0000) ? 1 : 0;
            R_H = (NNNN & 0x0000FF00) >> 8;
            R_L = (NNNN & 0x000000FF);
            break;
        case 0x3A: // LDD A, (HL)
            R_A = READ(R_HL);
            NN = R_HL - 1;
            R_H = NN >> 8;
            R_L = NN & 0xFF;
            break;
        case 0x3B: // DEC SP
            SP--;
            break;
        case 0x3C: // INC A
            R_A++;
            F_Z = (R_A == 0x00);
            F_N = 0;
            F_H = ((R_A & 0x0F) == 0x00);
            break;
        case 0x3D: // DEC A
            R_A--;
            F_Z = (R_A == 0x00);
            F_N = 1;
            F_H = ((R_A & 0x0F) == 0x0F);
            break;
        case 0x3E: // LD A, imm
            R_A = READ(PC++);
            break;
        case 0x3F: // CCF
            F_N = 0;
            F_H = 0;
            F_C = F_C ^ 0x1;
            break;
        case 0x40: // LD B, B
            break;
        case 0x41: // LD B, C
            R_B = R_C;
            break;
        case 0x42: // LD B, D
            R_B = R_D;
            break;
        case 0x43: // LD B, E
            R_B = R_E;
            break;
        case 0x44: // LD B, H
            R_B = R_H;
            break;
        case 0x45: // LD B, L
            R_B = R_L;
            break;
        case 0x46: // LD B, (HL)
            R_B = READ(R_HL);
            break;
        case 0x47: // LD B, A
            R_B = R_A;
            break;
        case 0x48: // LD C, B
            R_C = R_B;
            break;
        case 0x49: // LD C, C
            break;
        case 0x4A: // LD C, D
            R_C = R_D;
            break;
        case 0x4B: // LD C, E
            R_C = R_E;
            break;
        case 0x4C: // LD C, H
            R_C = R_H;
            break;
        case 0x4D: // LD C, L
            R_C = R_L;
            break;
        case 0x4E: // LD C, (HL)
            R_C = READ(R_HL);
            break;
        case 0x4F: // LD C, A
            R_C = R_A;
            break;
        case 0x50: // LD D, B
            R_D = R_B;
            break;
        case 0x51: // LD D, C
            R_D = R_C;
            break;
        case 0x52: // LD D, D
            break;
        case 0x53: // LD D, E
            R_D = R_E;
            break;
        case 0x54: // LD D, H
            R_D = R_H;
            break;
        case 0x55: // LD D, L
            R_D = R_L;
            break;
        case 0x56: // LD D, (HL)
            R_D = READ(R_HL);
            break;
        case 0x57: // LD D, A
            R_D = R_A;
            break;
        case 0x58: // LD E, B
            R_E = R_B;
            break;
        case 0x59: // LD E, C
            R_E = R_C;
            break;
        case 0x5A: // LD E, D
            R_E = R_D;
            break;
        case 0x5B: // LD E, E
            break;
        case 0x5C: // LD E, H
            R_E = R_H;
            break;
        case 0x5D: // LD E, L
            R_E = R_L;
            break;
        case 0x5E: // LD E, (HL)
            R_E = READ(R_HL);
            break;
        case 0x5F: // LD E, A
            R_E = R_A;
            break;
        case 0x60: // LD H, B
            R_H = R_B;
            break;
        case 0x61: // LD H, C
            R_H = R_C;
            break;
        case 0x62: // LD H, D
            R_H = R_D;
            break;
        case 0x63: // LD H, E
            R_H = R_E;
            break;
        case 0x64: // LD H, H
            break;
        case 0x65: // LD H, L
            R_H = R_L;
            break;
        case 0x66: // LD H, (HL)
            R_H = READ(R_HL);
            break;
        case 0x67: // LD H, A
            R_H = R_A;
            break;
        case 0x68: // LD L, B
            R_L = R_B;
            break;
        case 0x69: // LD L, C
            R_L = R_C;
            break;
        case 0x6A: // LD L, D
            R_L = R_D;
            break;
        case 0x6B: // LD L, E
            R_L = R_E;
            break;
        case 0x6C: // LD L, H
            R_L = R_H;
            break;
        case 0x6D: // LD L, L
            break;
        case 0x6E: // LD L, (HL)
            R_L = READ(R_HL);
            break;
        case 0x6F: // LD L, A
            R_L = R_A;
            break;
        case 0x70: // LD (HL), B
            WRITE(R_HL, R_B);
            break;
        case 0x71: // LD (HL), C
            WRITE(R_HL, R_C);
            break;
        case 0x72: // LD (HL), D
            WRITE(R_HL, R_D);
            break;
        case 0x73: // LD (HL), E
            WRITE(R_HL, R_E);
            break;
        case 0x74: // LD (HL), H
            WRITE(R_HL, R_H);
            break;
        case 0x75: // LD (HL), L
            WRITE(R_HL, R_L);
            break;
        case 0x76: // HALT
            gb_halt = 1;
            break;
        case 0x77: // LD (HL), A
            WRITE(R_HL, R_A);
            break;
        case 0x78: // LD A, B
            R_A = R_B;
            break;
        case 0x79: // LD A, C
            R_A = R_C;
            break;
        case 0x7A: // LD A, D
            R_A = R_D;
            break;
        case 0x7B: // LD A, E
            R_A = R_E;
            break;
        case 0x7C: // LD A, H
            R_A = R_H;
            break;
        case 0x7D: // LD A, L
            R_A = R_L;
            break;
        case 0x7E: // LD A, (HL)
            R_A = READ(R_HL);
            break;
        case 0x7F: // LD A, A
            break;
        case 0x80: // ADD A, B
            NN = R_A + R_B;
            F_Z = ((NN & 0xFF) == 0x00);
            F_N = 0;
            F_H = (R_A ^ R_B ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            R_A = NN & 0xFF;
            break;
        case 0x81: // ADD A, C
            NN = R_A + R_C;
            F_Z = ((NN & 0xFF) == 0x00);
            F_N = 0;
            F_H = (R_A ^ R_C ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            R_A = NN & 0xFF;
            break;
        case 0x82: // ADD A, D
            NN = R_A + R_D;
            F_Z = ((NN & 0xFF) == 0x00);
            F_N = 0;
            F_H = (R_A ^ R_D ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            R_A = NN & 0xFF;
            break;
        case 0x83: // ADD A, E
            NN = R_A + R_E;
            F_Z = ((NN & 0xFF) == 0x00);
            F_N = 0;
            F_H = (R_A ^ R_E ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            R_A = NN & 0xFF;
            break;
        case 0x84: // ADD A, H
            NN = R_A + R_H;
            F_Z = ((NN & 0xFF) == 0x00);
            F_N = 0;
            F_H = (R_A ^ R_H ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            R_A = NN & 0xFF;
            break;
        case 0x85: // ADD A, L
            NN = R_A + R_L;
            F_Z = ((NN & 0xFF) == 0x00);
            F_N = 0;
            F_H = (R_A ^ R_L ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            R_A = NN & 0xFF;
            break;
        case 0x86: // ADD A, (HL)
            N = READ(R_HL);
            NN = R_A + N;
            F_Z = ((NN & 0xFF) == 0x00);
            F_N = 0;
            F_H = (R_A ^ N ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            R_A = NN & 0xFF;
            break;
        case 0x87: // ADD A, A
            NN = R_A + R_A;
            F_Z = ((NN & 0xFF) == 0x00);
            F_N = 0;
            F_H = NN & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            R_A = NN & 0xFF;
            break;
        case 0x88: // ADC A, B
            N = R_B;
            NN = R_A + N + F_C;
            F_Z = ((NN & 0xFF) == 0x00);
            F_N = 0;
            F_H = (R_A ^ N ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            R_A = NN & 0xFF;
            break;
        case 0x89: // ADC A, C
            N = R_C;
            NN = R_A + N + F_C;
            F_Z = ((NN & 0xFF) == 0x00);
            F_N = 0;
            F_H = (R_A ^ N ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            R_A = NN & 0xFF;
            break;
        case 0x8A: // ADC A, D
            N = R_D;
            NN = R_A + N + F_C;
            F_Z = ((NN & 0xFF) == 0x00);
            F_N = 0;
            F_H = (R_A ^ N ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            R_A = NN & 0xFF;
            break;
        case 0x8B: // ADC A, E
            N = R_E;
            NN = R_A + N + F_C;
            F_Z = ((NN & 0xFF) == 0x00);
            F_N = 0;
            F_H = (R_A ^ N ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            R_A = NN & 0xFF;
            break;
        case 0x8C: // ADC A, H
            N = R_H;
            NN = R_A + N + F_C;
            F_Z = ((NN & 0xFF) == 0x00);
            F_N = 0;
            F_H = (R_A ^ N ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            R_A = NN & 0xFF;
            break;
        case 0x8D: // ADC A, L
            N = R_L;
            NN = R_A + N + F_C;
            F_Z = ((NN & 0xFF) == 0x00);
            F_N = 0;
            F_H = (R_A ^ N ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            R_A = NN & 0xFF;
            break;
        case 0x8E: // ADC A, (HL)
            N = READ(R_HL);
            NN = R_A + N + F_C;
            F_Z = ((NN & 0xFF) == 0x00);
            F_N = 0;
            F_H = (R_A ^ N ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            R_A = NN & 0xFF;
            break;
        case 0x8F: // ADC A, A
            N = R_A;
            NN = R_A + N + F_C;
            F_Z = ((NN & 0xFF) == 0x00);
            F_N = 0;
            F_H = (R_A ^ N ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            R_A = NN & 0xFF;
            break;
        case 0x90: // SUB B
            NN = R_A - R_B;
            F_Z = ((NN & 0xFF) == 0x00);
            F_N = 1;
            F_H = (R_A ^ R_B ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            R_A = NN & 0xFF;
            break;
        case 0x91: // SUB C
            NN = R_A - R_C;
            F_Z = ((NN & 0xFF) == 0x00);
            F_N = 1;
            F_H = (R_A ^ R_C ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            R_A = NN & 0xFF;
            break;
        case 0x92: // SUB D
            NN = R_A - R_D;
            F_Z = ((NN & 0xFF) == 0x00);
            F_N = 1;
            F_H = (R_A ^ R_D ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            R_A = NN & 0xFF;
            break;
        case 0x93: // SUB E
            NN = R_A - R_E;
            F_Z = ((NN & 0xFF) == 0x00);
            F_N = 1;
            F_H = (R_A ^ R_E ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            R_A = NN & 0xFF;
            break;
        case 0x94: // SUB H
            NN = R_A - R_H;
            F_Z = ((NN & 0xFF) == 0x00);
            F_N = 1;
            F_H = (R_A ^ R_H ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            R_A = NN & 0xFF;
            break;
        case 0x95: // SUB L
            NN = R_A - R_L;
            F_Z = ((NN & 0xFF) == 0x00);
            F_N = 1;
            F_H = (R_A ^ R_L ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            R_A = NN & 0xFF;
            break;
        case 0x96: // SUB (HL)
            N = READ(R_HL);
            NN = R_A - N;
            F_Z = ((NN & 0xFF) == 0x00);
            F_N = 1;
            F_H = (R_A ^ N ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            R_A = NN & 0xFF;
            break;
        case 0x97: // SUB A
            R_A = 0x00;
            F_Z = 1;
            F_N = 1;
            F_H = 0;
            F_C = 0;
            break;
        case 0x98: // SBC A, B
            N = R_B;
            NN = R_A - N - F_C;
            F_Z = ((NN & 0xFF) == 0x00);
            F_N = 1;
            F_H = (R_A ^ N ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            R_A = NN & 0xFF;
            break;
        case 0x99: // SBC A, C
            N = R_C;
            NN = R_A - N - F_C;
            F_Z = ((NN & 0xFF) == 0x00);
            F_N = 1;
            F_H = (R_A ^ N ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            R_A = NN & 0xFF;
            break;
        case 0x9A: // SBC A, D
            N = R_D;
            NN = R_A - N - F_C;
            F_Z = ((NN & 0xFF) == 0x00);
            F_N = 1;
            F_H = (R_A ^ N ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            R_A = NN & 0xFF;
            break;
        case 0x9B: // SBC A, E
            N = R_E;
            NN = R_A - N - F_C;
            F_Z = ((NN & 0xFF) == 0x00);
            F_N = 1;
            F_H = (R_A ^ N ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            R_A = NN & 0xFF;
            break;
        case 0x9C: // SBC A, H
            N = R_H;
            NN = R_A - N - F_C;
            F_Z = ((NN & 0xFF) == 0x00);
            F_N = 1;
            F_H = (R_A ^ N ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            R_A = NN & 0xFF;
            break;
        case 0x9D: // SBC A, L
            N = R_L;
            NN = R_A - N - F_C;
            F_Z = ((NN & 0xFF) == 0x00);
            F_N = 1;
            F_H = (R_A ^ N ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            R_A = NN & 0xFF;
            break;
        case 0x9E: // SBC A, (HL)
            N = READ(R_HL);
            NN = R_A - N - F_C;
            F_Z = ((NN & 0xFF) == 0x00);
            F_N = 1;
            F_H = (R_A ^ N ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            R_A = NN & 0xFF;
            break;
        case 0x9F: // SBC A, A
            R_A = F_C ? 0xFF : 0x00;
            F_Z = F_C ? 0x00 : 0x01;
            F_N = 1;
            F_H = F_C;
            break;
        case 0xA0: // AND B
            R_A = R_A & R_B;
            F_Z = (R_A == 0x00);
            F_N = 0;
            F_H = 1;
            F_C = 0;
            break;
        case 0xA1: // AND C
            R_A = R_A & R_C;
            F_Z = (R_A == 0x00);
            F_N = 0;
            F_H = 1;
            F_C = 0;
            break;
        case 0xA2: // AND D
            R_A = R_A & R_D;
            F_Z = (R_A == 0x00);
            F_N = 0;
            F_H = 1;
            F_C = 0;
            break;
        case 0xA3: // AND E
            R_A = R_A & R_E;
            F_Z = (R_A == 0x00);
            F_N = 0;
            F_H = 1;
            F_C = 0;
            break;
        case 0xA4: // AND H
            R_A = R_A & R_H;
            F_Z = (R_A == 0x00);
            F_N = 0;
            F_H = 1;
            F_C = 0;
            break;
        case 0xA5: // AND L
            R_A = R_A & R_L;
            F_Z = (R_A == 0x00);
            F_N = 0;
            F_H = 1;
            F_C = 0;
            break;
        case 0xA6: // AND (HL)
            R_A = R_A & READ(R_HL);
            F_Z = (R_A == 0x00);
            F_N = 0;
            F_H = 1;
            F_C = 0;
            break;
        case 0xA7: // AND A
            F_Z = (R_A == 0x00);
            F_N = 0;
            F_H = 1;
            F_C = 0;
            break;
        case 0xA8: // XOR B
            R_A = R_A ^ R_B;
            F_Z = (R_A == 0x00);
            F_N = 0;
            F_H = 0;
            F_C = 0;
            break;
        case 0xA9: // XOR C
            R_A = R_A ^ R_C;
            F_Z = (R_A == 0x00);
            F_N = 0;
            F_H = 0;
            F_C = 0;
            break;
        case 0xAA: // XOR D
            R_A = R_A ^ R_D;
            F_Z = (R_A == 0x00);
            F_N = 0;
            F_H = 0;
            F_C = 0;
            break;
        case 0xAB: // XOR E
            R_A = R_A ^ R_E;
            F_Z = (R_A == 0x00);
            F_N = 0;
            F_H = 0;
            F_C = 0;
            break;
        case 0xAC: // XOR H
            R_A = R_A ^ R_H;
            F_Z = (R_A == 0x00);
            F_N = 0;
            F_H = 0;
            F_C = 0;
            break;
        case 0xAD: // XOR L
            R_A = R_A ^ R_L;
            F_Z = (R_A == 0x00);
            F_N = 0;
            F_H = 0;
            F_C = 0;
            break;
        case 0xAE: // XOR (HL)
            R_A = R_A ^ READ(R_HL);
            F_Z = (R_A == 0x00);
            F_N = 0;
            F_H = 0;
            F_C = 0;
            break;
        case 0xAF: // XOR A
            R_A = 0x00;
            F_Z = 1;
            F_N = 0;
            F_H = 0;
            F_C = 0;
            break;
        case 0xB0: // OR B
            R_A = R_A | R_B;
            F_Z = (R_A == 0x00);
            F_N = 0;
            F_H = 0;
            F_C = 0;
            break;
        case 0xB1: // OR C
            R_A = R_A | R_C;
            F_Z = (R_A == 0x00);
            F_N = 0;
            F_H = 0;
            F_C = 0;
            break;
        case 0xB2: // OR D
            R_A = R_A | R_D;
            F_Z = (R_A == 0x00);
            F_N = 0;
            F_H = 0;
            F_C = 0;
            break;
        case 0xB3: // OR E
            R_A = R_A | R_E;
            F_Z = (R_A == 0x00);
            F_N = 0;
            F_H = 0;
            F_C = 0;
            break;
        case 0xB4: // OR H
            R_A = R_A | R_H;
            F_Z = (R_A == 0x00);
            F_N = 0;
            F_H = 0;
            F_C = 0;
            break;
        case 0xB5: // OR L
            R_A = R_A | R_L;
            F_Z = (R_A == 0x00);
            F_N = 0;
            F_H = 0;
            F_C = 0;
            break;
        case 0xB6: // OR (HL)
            R_A = R_A | READ(R_HL);
            F_Z = (R_A == 0x00);
            F_N = 0;
            F_H = 0;
            F_C = 0;
            break;
        case 0xB7: // OR A
            F_Z = (R_A == 0x00);
            F_N = 0;
            F_H = 0;
            F_C = 0;
            break;
        case 0xB8: // CP B
            NN = R_A - R_B;
            F_Z = ((NN & 0xFF) == 0x00);
            F_N = 1;
            F_H = (R_A ^ R_B ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            break;
        case 0xB9: // CP C
            NN = R_A - R_C;
            F_Z = ((NN & 0xFF) == 0x00);
            F_N = 1;
            F_H = (R_A ^ R_C ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            break;
        case 0xBA: // CP D
            NN = R_A - R_D;
            F_Z = ((NN & 0xFF) == 0x00);
            F_N = 1;
            F_H = (R_A ^ R_D ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            break;
        case 0xBB: // CP E
            NN = R_A - R_E;
            F_Z = ((NN & 0xFF) == 0x00);
            F_N = 1;
            F_H = (R_A ^ R_E ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            break;
        case 0xBC: // CP H
            NN = R_A - R_H;
            F_Z = ((NN & 0xFF) == 0x00);
            F_N = 1;
            F_H = (R_A ^ R_H ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            break;
        case 0xBD: // CP L
            NN = R_A - R_L;
            F_Z = ((NN & 0xFF) == 0x00);
            F_N = 1;
            F_H = (R_A ^ R_L ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            break;
        case 0xBE: // CP (HL)
            N = READ(R_HL);
            NN = R_A - N;
            F_Z = ((NN & 0xFF) == 0x00);
            F_N = 1;
            F_H = (R_A ^ N ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            break;
        case 0xBF: // CP A
            F_Z = 1;
            F_N = 1;
            F_H = 0;
            F_C = 0;
            break;
        case 0xC0: // RET NZ
            if (!F_Z)
                {
                NN = READ(SP++);
                NN |= READ(SP++) << 8;
                PC = NN;
                }
            break;
        case 0xC1: // POP BC
            R_C = READ(SP++);
            R_B = READ(SP++);
            break;
        case 0xC2: // JP NZ, imm
            NN = READ(PC++);
            NN |= READ(PC++) << 8;
            if (!F_Z)
                {
                PC = NN;
                }
            break;
        case 0xC3: // JP imm
            NN = READ(PC++);
            NN |= READ(PC) << 8;
            PC = NN;
            break;
        case 0xC4: // CALL NZ, imm
            NN = READ(PC++);
            NN |= READ(PC++) << 8;
            if (!F_Z)
                {
                WRITE(--SP, PC >> 8);
                WRITE(--SP, PC & 0xFF);
                PC = NN;
                }
            break;
        case 0xC5: // PUSH BC
            WRITE(--SP, R_B);
            WRITE(--SP, R_C);
            break;
        case 0xC6: // ADD A, imm
            N = READ(PC++);
            NN = R_A + N;
            F_Z = ((NN & 0xFF) == 0x00);
            F_N = 0;
            F_H = (R_A ^ N ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            R_A = NN & 0xFF;
            break;
        case 0xC7: // RST 0x0000
            WRITE(--SP, PC >> 8);
            WRITE(--SP, PC & 0xFF);
            PC = 0x0000;
            break;
        case 0xC8: // RET Z
            if (F_Z)
                {
                NN = READ(SP++);
                NN |= READ(SP++) << 8;
                PC = NN;
                }
            break;
        case 0xC9: // RET
            NN = READ(SP++);
            NN |= READ(SP++) << 8;
            PC = NN;
            break;
        case 0xCA: // JP Z, imm
            NN = READ(PC++);
            NN |= READ(PC++) << 8;
            if (F_Z)
                {
                PC = NN;
                }
            break;
        case 0xCB: // CB INST
            ExecuteCB();
            break;
        case 0xCC: // CALL Z, imm
            NN = READ(PC++);
            NN |= READ(PC++) << 8;
            if (F_Z)
                {
                WRITE(--SP, PC >> 8);
                WRITE(--SP, PC & 0xFF);
                PC = NN;
                }
            break;
        case 0xCD: // CALL imm
            NN = READ(PC++);
            NN |= READ(PC++) << 8;
            WRITE(--SP, PC >> 8);
            WRITE(--SP, PC & 0xFF);
            PC = NN;
            break;
        case 0xCE: // ADC A, imm
            N = READ(PC++);
            NN = R_A + N + F_C;
            F_Z = ((NN & 0xFF) == 0x00);
            F_N = 0;
            F_H = (R_A ^ N ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            R_A = NN & 0xFF;
            break;
        case 0xCF: // RST 0x0008
            WRITE(--SP, PC >> 8);
            WRITE(--SP, PC & 0xFF);
            PC = 0x0008;
            break;
        case 0xD0: // RET NC
            if (!F_C)
                {
                NN = READ(SP++);
                NN |= READ(SP++) << 8;
                PC = NN;
                }
            break;
        case 0xD1: // POP DE
            R_E = READ(SP++);
            R_D = READ(SP++);
            break;
        case 0xD2: // JP NC, imm
            NN = READ(PC++);
            NN |= READ(PC++) << 8;
            if (!F_C)
                {
                PC = NN;
                }
            break;
        case 0xD3: // illegal
            break;
        case 0xD4: // CALL NC, imm
            NN = READ(PC++);
            NN |= READ(PC++) << 8;
            if (!F_C)
                {
                WRITE(--SP, PC >> 8);
                WRITE(--SP, PC & 0xFF);
                PC = NN;
                }
            break;
        case 0xD5: // PUSH DE
            WRITE(--SP, R_D);
            WRITE(--SP, R_E);
            break;
        case 0xD6: // SUB imm
            N = READ(PC++);
            NN = R_A - N;
            F_Z = ((NN & 0xFF) == 0x00);
            F_N = 1;
            F_H = (R_A ^ N ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            R_A = NN & 0xFF;
            break;
        case 0xD7: // RST 0x0010
            WRITE(--SP, PC >> 8);
            WRITE(--SP, PC & 0xFF);
            PC = 0x0010;
            break;
        case 0xD8: // RET C
            if (F_C)
                {
                NN = READ(SP++);
                NN |= READ(SP++) << 8;
                PC = NN;
                }
            break;
        case 0xD9: // RETI
            NN = READ(SP++);
            NN |= READ(SP++) << 8;
            PC = NN;
            gb_ime = 1;
            break;
        case 0xDA: // JP C, imm
            NN = READ(PC++);
            NN |= READ(PC++) << 8;
            if (F_C)
                {
                PC = NN;
                }
            break;
        case 0xDB: // illegal
            break;
        case 0xDC: // CALL C, imm
            NN = READ(PC++);
            NN |= READ(PC++) << 8;
            if (F_C)
                {
                WRITE(--SP, PC >> 8);
                WRITE(--SP, PC & 0xFF);
                PC = NN;
                }
            break;
        case 0xDD: // illegal
            break;
        case 0xDE: // SBC A, imm
            N = READ(PC++);
            NN = R_A - N - F_C;
            F_Z = ((NN & 0xFF) == 0x00);
            F_N = 1;
            F_H = (R_A ^ N ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            R_A = NN & 0xFF;
            break;
        case 0xDF: // RST 0x0018
            WRITE(--SP, PC >> 8);
            WRITE(--SP, PC & 0xFF);
            PC = 0x0018;
            break;
        case 0xE0: // LD (0xFF00+imm), A
            WRITE(0xFF00 | READ(PC++), R_A);
            break;
        case 0xE1: // POP HL
            R_L = READ(SP++);
            R_H = READ(SP++);
            break;
        case 0xE2: // LD (C), A
            WRITE(0xFF00 | R_C, R_A);
            break;
        case 0xE3: // illegal
            break;
        case 0xE4: // illegal
            break;
        case 0xE5: // PUSH HL
            WRITE(--SP, R_H);
            WRITE(--SP, R_L);
            break;
        case 0xE6: // AND imm
            R_A = R_A & READ(PC++);
            F_Z = (R_A == 0x00);
            F_N = 0;
            F_H = 1;
            F_C = 0;
            break;
        case 0xE7: // RST 0x0020
            WRITE(--SP, PC >> 8);
            WRITE(--SP, PC & 0xFF);
            PC = 0x0020;
            break;
        case 0xE8: // ADD SP, imm
            SN = (s8)READ(PC++);
            NN = SP + SN;
            if (SN >= 0)
                {
                F_Z = 0;
                F_N = 0;
                F_H = ((SP ^ SN ^ NN) & 0x1000) ? 1 : 0;
                F_C = (SP > NN);
                }
            else
                {
                F_Z = 0;
                F_N = 0;
                F_H = ((SP ^ SN ^ NN) & 0x1000) ? 1 : 0;
                F_C = (SP < NN);
                }
            SP = NN;
            break;
        case 0xE9: // JP (HL)
            PC = R_HL;
            break;
        case 0xEA: // LD (imm), A
            WRITE(READ(PC) | READ(PC+1) << 8, R_A);
            PC += 2;
            break;
        case 0xEB: // illegal
            break;
        case 0xEC: // illegal
            break;
        case 0xED: // illegal
            break;
        case 0xEE: // XOR imm
            R_A = R_A ^ READ(PC++);
            F_Z = (R_A == 0x00);
            F_N = 0;
            F_H = 0;
            F_C = 0;
            break;
        case 0xEF: // RST 0x0028
            WRITE(--SP, PC >> 8);
            WRITE(--SP, PC & 0xFF);
            PC = 0x0028;
            break;
        case 0xF0: // LD A, (0xFF00+imm)
            R_A = READ(0xFF00 | READ(PC++));
            break;
        case 0xF1: // POP AF
            N = READ(SP++);
            F_Z = (N >> 7) & 1;
            F_N = (N >> 6) & 1;
            F_H = (N >> 5) & 1;
            F_C = (N >> 4) & 1;
            R_A = READ(SP++);
            break;
        case 0xF2: // LD A, (C)
            R_A = READ(0xFF00 | R_C);
            break;
        case 0xF3: // DI
            gb_ime = 0;
            break;
        case 0xF4: // illegal
            break;
        case 0xF5: // PUSH AF
            WRITE(--SP, R_A);
            WRITE(--SP, F_Z << 7 | F_N << 6 | F_H << 5 | F_C << 4);
            break;
        case 0xF6: // OR imm
            R_A = R_A | READ(PC++);
            F_Z = (R_A == 0x00);
            F_N = 0;
            F_H = 0;
            F_C = 0;
            break;
        case 0xF7: // RST 0x0030
            WRITE(--SP, PC >> 8);
            WRITE(--SP, PC & 0xFF);
            PC = 0x0030;
            break;
        case 0xF8: // LD HL, SP+/-imm
            SN = (s8)READ(PC++);
            NN = SP + SN;
            if (SN >= 0)
                {
                F_Z = 0;
                F_N = 0;
                F_H = ((SP ^ SN ^ NN) & 0x1000) ? 1 : 0;
                F_C = (SP > NN);
                }
            else
                {
                F_Z = 0;
                F_N = 0;
                F_H = ((SP ^ SN ^ NN) & 0x1000) ? 1 : 0;
                F_C = (SP < NN);
                }
            R_H = (NN & 0xFF00) >> 8;
            R_L = (NN & 0x00FF);
            break;
        case 0xF9: // LD SP, HL
            SP = R_HL;
            break;
        case 0xFA: // LD A, (imm)
            R_A = READ(READ(PC) | READ(PC+1) << 8);
            PC += 2;
            break;
        case 0xFB: // EI
            gb_ime = 1;
            break;
        case 0xFC: // illegal
            break;
        case 0xFD: // illegal
            break;
        case 0xFE: // CP imm
            N = READ(PC++);
            NN = R_A - N;
            F_Z = ((NN & 0xFF) == 0x00);
            F_N = 1;
            F_H = (R_A ^ N ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            break;
        case 0xFF: // RST 0x0038
            WRITE(--SP, PC >> 8);
            WRITE(--SP, PC & 0xFF);
            PC = 0x0038;
            break;
        }
    
    // CPU timing
    cpu_count += inst_cycles;
    
    // AUDIO timing
    audio_count += inst_cycles;
    if (audio_count > AUDIO_CYCLES)
        {
        AudioUpdate();
        audio_count -= AUDIO_CYCLES;
        }

    // LCD timing
    lcd_count += inst_cycles;
    // New scanline
    if (lcd_count > LCD_LINE_CYCLES * (cgb_double + 1))
        {
        lcd_count -= LCD_LINE_CYCLES * (cgb_double + 1);

        // LYC update
        if (R_LY == R_LYC)
            {
            R_STAT |= STAT_LYC_COINC;
            if (R_STAT & STAT_LYC_INTR)
                R_IF |= LCDC_INTR;
            }
        else
            R_STAT &= 0xFB;

        // next line
        R_LY = (R_LY + 1) % LCD_VERT_LINES;
        
        // VBLANK Start
        if (R_LY == LCD_HEIGHT)
            {
            //OutputDebugString("---- VBLANK ----\n");
            lcd_mode = LCD_VBLANK;
            gb_frame = 1;
            
            R_IF |= VBLANK_INTR;
            if (R_STAT & STAT_MODE_1_INTR)
                R_IF |= LCDC_INTR;
            }
        // normal line
        else if (R_LY < LCD_HEIGHT)
            {
            if (R_LY == 0)
                LCDStart();
            lcd_mode = LCD_HBLANK;
            if (R_STAT & STAT_MODE_0_INTR)
                R_IF |= LCDC_INTR;

            // check for HBLANK HDMA
            if (cgb_enable && (R_HDMA & HDMA_OFF) == 0x00)
                {
                u8 i;
                // copy 0x10 bytes each hblank
                for (i = 0; i < 0x10; i++)
                    WRITE(R_HDMAD + (R_HDMA * 0x10) + i, READ(R_HDMAS + (R_HDMA * 0x10) + i));
                R_HDMA--;
                lcd_count += 8 * (cgb_double + 1);
                inst_cycles += 8 * (cgb_double + 1);
                }
            }
        }
    // oam access
    else if (lcd_mode == LCD_HBLANK && lcd_count >= LCD_MODE_2_CYCLES * (cgb_double + 1))
        {
        lcd_mode = LCD_SEARCH_OAM;
        if (R_STAT & STAT_MODE_2_INTR)
            R_IF |= LCDC_INTR;
        }        
    // update LCD
    else if (lcd_mode == LCD_SEARCH_OAM && lcd_count >= LCD_MODE_3_CYCLES * (cgb_double + 1))
        {
        lcd_mode = LCD_TRANSFER;
        LCDDrawLine();
        }
        
    // DIV register timing
    div_count += inst_cycles;
    if (div_count > DIV_CYCLES)
        {
        R_DIV++;
        div_count -= DIV_CYCLES;
        }
    
    // TIMA register timing
    if (tac_enable)
        {
        tima_count += inst_cycles;
        if (tima_count > TAC_CYCLES[tac_rate])
            {
            tima_count -= TAC_CYCLES[tac_rate];
            R_TIMA++;
            if (R_TIMA == 0)
                {
                R_IF |= TIMER_INTR;
                R_TIMA = R_TMA;
                }
            }
        }
    }

// CB helper variables
u8 CBOP;
u8 R;
u8 B;
u8 D;
u8 val;
u8 writeback;

// bit-level operations
void ExecuteCB()
    {
    CBOP = READ(PC++);
    R  = (CBOP & 0x7);
    B  = (CBOP >> 3) & 0x7;
    D  = (CBOP >> 3) & 0x1;
    
    // retrieve byte to manipulate
    switch (R)
        {
        case 0: val = R_B; break;
        case 1: val = R_C; break;
        case 2: val = R_D; break;
        case 3: val = R_E; break;
        case 4: val = R_H; break;
        case 5: val = R_L; break;
        case 6: val = READ(R_HL); break;
        case 7: val = R_A; break;
        }
    
    // bit-fiddling OPs
    writeback = 1;
    switch (CBOP >> 6)
        {
        case 0x0:
            CBOP = (CBOP >> 4) & 0x3;
            switch (CBOP)
                {
                case 0x0: // RdC R
                case 0x1: // Rd R
                    if (D) // RRC R / RR R
                        {
                        N = val;
                        val = (val >> 1);
                        val |= CBOP ? (F_C << 7) : (N << 7);
                        F_Z = (val == 0x00);
                        F_N = 0;
                        F_H = 0;
                        F_C = (N & 0x01);
                        }
                    else    // RLC R / RL R
                        {
                        N = val;
                        val = (val << 1);
                        val |= CBOP ? F_C : (N >> 7);
                        F_Z = (val == 0x00);
                        F_N = 0;
                        F_H = 0;
                        F_C = (N >> 7);
                        }
                    break;
                case 0x2:
                    if (D) // SRA R
                        {
                        F_C = val & 0x01;
                        val = (val >> 1) | (val & 0x80);
                        F_Z = (val == 0x00);
                        F_N = 0;
                        F_H = 0;
                        }
                    else    // SLA R
                        {
                        F_C = (val >> 7);
                        val = val << 1;
                        F_Z = (val == 0x00);
                        F_N = 0;
                        F_H = 0;
                        }
                    break;
                case 0x3:
                    if (D) // SRL R
                        {
                        F_C = val & 0x01;
                        val = val >> 1;
                        F_Z = (val == 0x00);
                        F_N = 0;
                        F_H = 0;
                        }
                    else    // SWAP R
                        {
                        N = (val >> 4) & 0x0F;
                        N |= (val << 4) & 0xF0;
                        val = N;
                        F_Z = (val == 0);
                        F_N = 0;
                        F_H = 0;
                        F_C = 0;
                        }
                    break;
                }
            break;
        case 0x1: // BIT B, R
            F_Z = !((val >> B) & 0x1);
            F_N = 0;
            F_H = 1;
            writeback = 0;
            break;
        case 0x2: // RES B, R
            val &= (0xFE << B) | (0xFF >> (8 - B));
            break;
        case 0x3: // SET B, R
            val |= (0x1 << B);
            break;
        }
        
    // save result
    if (writeback)
        {
        switch (R) 
            {
            case 0: R_B = val; break;
            case 1: R_C = val; break;
            case 2: R_D = val; break;
            case 3: R_E = val; break;
            case 4: R_H = val; break;
            case 5: R_L = val; break;
            case 6: WRITE(R_HL, val); break;
            case 7: R_A = val; break;
            }
        }
    }

// CONTROLS /////////////////////////
void UpdateP1()
    {
    R_P1 |= 0x0F;
    if (!(R_P1 & INPUT_P14))
        R_P1 &= 0xF0 | ((gb_keys & 0x0F) ^ 0x0F);
    if (!(R_P1 & INPUT_P15))
        R_P1 &= 0xF0 | (((gb_keys >> 4) & 0x0F) ^ 0x0F);
    }

void KeyPress(u8 key)
    {
    gb_keys |= 0x01 << key;
    UpdateP1();
    //R_IF |= CONTROL_INTR;
    }

void KeyRelease(u8 key)
    {
    gb_keys &= (0x01 << key) ^ 0xFF;
    UpdateP1();
    //R_IF |= CONTROL_INTR;
    }

// SOUND ////////////////////////////
// 

// GAMEBOY //////////////////////////
void LoadROM(u8* rom, u32 size, u8* save, u32 save_size)
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
                    ROM[ROM_CGB_SUPPORT] == CGB_OPTIONAL);

    PowerUp();
    }

u32 GetSaveSize(u8* rom)
    {
    u32 ram_sizes[] = {0x00, 0x800, 0x2000, 0x8000, 0x20000};
    return ram_sizes[rom[ROM_RAM_SIZE]];
    }

void PowerUp()
    {
    u32 i; u8 x; u8 y;
    // GB
    gb_halt = 0;
    gb_ime = 1;
    gb_bios_enable = !cgb_enable;
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

#ifdef DMG_BIOS_ENABLE
    if (gb_bios_enable && opt_use_gb_bios) 
    {
        PC  = 0x0000;
    } else {
#endif
        PC  = 0x0100;
        gb_bios_enable = 0;
#ifdef DMG_BIOS_ENABLE
    }
#endif
    
    // timer
    cpu_count   = 0x0000;
    lcd_count   = 0x0000;
    div_count   = 0x0000;
    tima_count  = 0x0000;
    tac_enable  = 0x00;
    tac_rate    = 0x00;
    
    // register initialization
    R_TIMA      = 0x00;
    R_TMA       = 0x00;
    R_TAC       = 0x00;
    // sound subsystem
    R_NR10      = 0x80;
    R_NR11      = 0xBF;
    R_NR12      = 0xF3;
    R_NR13      = 0xFF;
    R_NR14      = 0xBF;
    R_NR20      = 0xFF;
    R_NR21      = 0x3F;
    R_NR22      = 0x00;
    R_NR23      = 0xFF;
    R_NR24      = 0xBF;
    R_NR30      = 0x7F;
    R_NR31      = 0xFF;
    R_NR32      = 0x9F;
    R_NR33      = 0xBF;
    R_NR34      = 0xFF;
    R_NR40      = 0xFF;
    R_NR41      = 0xFF;
    R_NR42      = 0x00;
    R_NR43      = 0x00;
    R_NR44      = 0xBF;
    R_NR50      = 0x77;
    R_NR51      = 0xF3;
    R_NR52      = 0xF1;
    // display
    R_LCDC      = 0x91;
    R_SCY       = 0x00;
    R_SCX       = 0x00;
    R_LYC       = 0x00;
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
    for (i = 0; i < 32; i++)
        BCPD[i] = 0xFFFF;

    // GB framebuffer
    for (y = 0; y < LCD_HEIGHT; y++)
        for (x = 0; x < LCD_WIDTH; x++)
            gb_fb[y][x] = 0;

    // Clear VRAM
    for (i = 0; i < VRAM_SIZE; i++)
        {
        VRAM[i] = 0;
        }
    }

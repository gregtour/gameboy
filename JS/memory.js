
// Read
function READ(addr)
    {
    switch ((addr >> 12) & 0xF)
        {
        case 0x0:
            if (gb_bios_enable && addr < 0x100)
                {
                return DMG_BIOS[addr];
                }
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
            return VRAM[addr - VRAM_ADDR + vram_bank*VRAM_BANK_SIZE];
            
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
                return 0; // lol unused
            switch (addr & 0xFF)        // 0xFF00 .. 0xFFFF
                {
                // I/O Registers
                case 0x00: return 0xC0 | R_P1;
                case 0x01: return R_SB;
                case 0x02: return R_SC;
                    
                // Timer Registers                    
                case 0x04: return R_DIV;
                case 0x05: return R_TIMA;
                case 0x06: return R_TMA;
                case 0x07: return R_TAC;
                    
                // Interrupt Flag Register
                case 0x0F: return R_IF;
                    
                // Sound Registers (lol wut sound)
                case 0x10: return R_NR10;
                case 0x11: return R_NR11;
                case 0x12: return R_NR12;
                case 0x14: return R_NR14;
                case 0x16: return R_NR21;
                case 0x17: return R_NR22;
                case 0x19: return R_NR24;
                case 0x1A: return R_NR30;
                case 0x1B: return R_NR31;
                case 0x1C: return R_NR32;
                case 0x1E: return R_NR33;
                case 0x20: return R_NR41;
                case 0x21: return R_NR42;
                case 0x22: return R_NR43;
                case 0x23: return R_NR44;
                case 0x24: return R_NR50;
                case 0x25: return R_NR51;
                case 0x26: return R_NR52;
                
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
                case 0x69: return BCPD[R_BCPS & PAL_INDEX];
                case 0x6A: return R_OCPS;
                case 0x6B: return OCPD[R_OCPS & PAL_INDEX];

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
function WRITE(addr, val)
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
                return;    // lol unused
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
                case 0x10: R_NR10 = val;    return;
                case 0x11: R_NR11 = val;    return;
                case 0x12: R_NR12 = val;    return;
                case 0x14: R_NR14 = val;    return;
                case 0x16: R_NR21 = val;    return;
                case 0x17: R_NR22 = val;    return;
                case 0x19: R_NR24 = val;    return;
                case 0x1A: R_NR30 = val;    return;
                case 0x1B: R_NR31 = val;    return;
                case 0x1C: R_NR32 = val;    return;
                case 0x1E: R_NR33 = val;    return;
                case 0x20: R_NR41 = val;    return;
                case 0x21: R_NR42 = val;    return;
                case 0x22: R_NR43 = val;    return;
                case 0x23: R_NR44 = val;    return;
                case 0x24: R_NR50 = val;    return;
                case 0x25: R_NR51 = val;    return;
                case 0x26: R_NR52 = val;    return;
                
                // LCD Registers
                case 0x40: R_LCDC = val;    return;
                case 0x41: R_STAT = val;    return;
                case 0x42: R_SCY = val;     return;
                case 0x43: R_SCX = val;     return;
                case 0x44: R_LY = val;      return;
                case 0x45: R_LYC = val;     return;
                    
                // DMA Register
                case 0x46:
                    R_DMA = (val % 0xF1);
                    for (var i = 0; i < OAM_SIZE; i++)
                        OAM[i] = READ((R_DMA << 8) + i);
                    return;
                
                // DGM Palette Registers
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
                            // blocking DMA
                            for (var i = 0; i < (val + 1) * 0x10; i++)
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
                    BCPD[R_BCPS & PAL_INDEX] = val;
                    if (R_BCPS & PAL_AUTO_INCREMENT)
                        R_BCPS = PAL_CONTROL_BITS & (R_BCPS + 1);
                    return;
                case 0x6A: R_OCPS = val;    return;
                case 0x6B:
                    OCPD[R_OCPS & PAL_INDEX] = val;
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



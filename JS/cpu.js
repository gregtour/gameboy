
function RunFrame()
    {
    gb_frame = 0;
    while (!gb_frame)
        StepCPU();
    }

var N, SN, NN, NNNN;

function StepCPU()
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
	//if (PC == 0xE0) debugger;	
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
            WRITE((R_B << 8 | R_C), R_A);
            break;
        case 0x03: // INC BC
            NN = ((R_B << 8 | R_C) + 1) % 0x10000;
            R_B = NN >> 8;
            R_C = NN & 0xFF;
            break;
        case 0x04: // INC B
            R_B = (R_B + 1) % 0x100;
            F_Z = (R_B == 0x00) ? 1 : 0;
            F_N = 0;
            F_H = ((R_B & 0x0F) == 0x00) ? 1 : 0;
            break;
        case 0x05: // DEC B
            R_B = (R_B + 0xFF) % 0x100;
            F_Z = (R_B == 0x00) ? 1 : 0;
            F_N = 1;
            F_H = ((R_B & 0x0F) == 0x0F) ? 1 : 0;
            break;
        case 0x06: // LD B, imm
            R_B = READ(PC++);
            break;
        case 0x07: // RLCA
            R_A = ((R_A << 1) & 0xFF) | (R_A >> 7);
            F_Z = 0;            
            F_N = 0;
            F_H = 0;
            F_C = (R_A & 0x01);
            break;
        case 0x08: // LD (imm), SP
            NN = READ(PC++) | READ(PC++) << 8;
            WRITE(NN++, SP & 0xFF);
            WRITE(NN, SP >> 8);
            break;
        case 0x09: // ADD HL, BC
            NNNN = (R_H << 8 | R_L) + (R_B << 8 | R_C);
            F_N = 0;
            F_H = (NNNN ^ (R_H << 8 | R_L) ^ (R_B << 8 | R_C)) & 0x1000 ? 1 : 0;
            F_C = (NNNN & 0xFFFF0000) ? 1 : 0;
            R_H = (NNNN & 0x0000FF00) >> 8;
            R_L = (NNNN & 0x000000FF);
            break;
        case 0x0A: // LD A, (BC)
            R_A = READ((R_B << 8 | R_C));
            break;
        case 0x0B: // DEC BC
            NN = ((R_B << 8 | R_C) + 0xFFFF) % 0x10000;
            R_B = NN >> 8;
            R_C = NN & 0xFF;
            break;
        case 0x0C: // INC C
            R_C = (R_C + 1) % 0x100;
            F_Z = (R_C == 0x00) ? 1 : 0;
            F_N = 0;
            F_H = ((R_C & 0x0F) == 0x00) ? 1 : 0;
            break;
        case 0x0D: // DEC C
            R_C = (R_C + 0xFF) % 0x100;
            F_Z = (R_C == 0x00) ? 1 : 0;
            F_N = 1;
            F_H = ((R_C & 0x0F) == 0x0F) ? 1 : 0;
            break;
        case 0x0E: // LD C, imm
            R_C = READ(PC++);
            break;
        case 0x0F: // RRCA
            F_C = R_A & 0x01;
            R_A = (R_A >> 1) | ((R_A << 7) & 0x80);
            F_Z = 0;
            F_N = 0;
            F_H = 0;
            break;
        case 0x10: // STOP
            gb_halt = 1;
            if (cgb_enable)
                {
                // check for cgb speed change
                if (R_KEY1 & 0x01)
                    {
                    cgb_double = cgb_double ? 0 : 1;
                    R_KEY1 = 0x00;
                    }
                }
            break;
        case 0x11: // LD DE, imm
            R_E = READ(PC++);
            R_D = READ(PC++);
            break;
        case 0x12: // LD (DE), A
            WRITE((R_D << 8 | R_E), R_A);
            break;
        case 0x13: // INC DE
            NN = ((R_D << 8 | R_E) + 1) % 0x10000;
            R_D = NN >> 8;
            R_E = NN & 0xFF;
            break;
        case 0x14: // INC D
            R_D = (R_D + 1) % 0x100;
            F_Z = (R_D == 0x00) ? 1 : 0;
            F_N = 0;
            F_H =  ((R_D & 0x0F) == 0x00) ? 1 : 0;
            break;
        case 0x15: // DEC D
            R_D = (R_D + 0xFF) % 0x100;
            F_Z = (R_D == 0x00) ? 1 : 0;
            F_N = 1;
            F_H = ((R_D & 0x0F) == 0x0F) ? 1 : 0;
            break;
        case 0x16: // LD D, imm
            R_D = READ(PC++);
            break;
        case 0x17: // RLA
            N = R_A;
            R_A = (R_A << 1 | F_C) & 0xFF;
            F_Z = 0;            
            F_N = 0;
            F_H = 0;
            F_C = (N >> 7) & 0x01;
            break;
        case 0x18: // JR imm
            SN = READ(PC++);
			SN = (SN & 0x7F) - (SN & 0x80);
            PC += SN;
            break;
        case 0x19: // ADD HL, DE
            NNNN = (R_H << 8 | R_L) + (R_D << 8 | R_E);
            F_N = 0;
            F_H = (NNNN ^ (R_H << 8 | R_L) ^ (R_D << 8 | R_E)) & 0x1000 ? 1 : 0;
            F_C = (NNNN & 0xFFFF0000) ? 1 : 0;
            R_H = (NNNN & 0x0000FF00) >> 8;
            R_L = (NNNN & 0x000000FF);
            break;
        case 0x1A: // LD A, (DE)
            R_A = READ((R_D << 8 | R_E));
            break;
        case 0x1B: // DEC DE
            NN = ((R_D << 8 | R_E) + 0xFFFF) % 0x10000;
            R_D = NN >> 8;
            R_E = NN & 0xFF;
            break;
        case 0x1C: // INC E
            R_E = (R_E + 1) % 0x100;
            F_Z = (R_E == 0x00) ? 1 : 0;
            F_N = 0;
            F_H = ((R_E & 0x0F) == 0x00) ? 1 : 0;
            break;
        case 0x1D: // DEC E
            R_E = (R_E + 0xFF) % 0x100;
            F_Z = (R_E == 0x00) ? 1 : 0;
            F_N = 1;
            F_H = ((R_E & 0x0F) == 0x0F) ? 1 : 0;
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
            SN = READ(PC++);
			SN = (SN & 0x7F) - (SN & 0x80);
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
            WRITE((R_H << 8 | R_L), R_A);
            NN = ((R_H << 8 | R_L) + 1) % 0x10000;
            R_H = NN >> 8;
            R_L = NN & 0xFF;
            break;
        case 0x23: // INC HL
            NN = ((R_H << 8 | R_L) + 1) % 0x10000;
            R_H = NN >> 8;
            R_L = NN & 0xFF;
            break;
        case 0x24: // INC H
            R_H = (R_H + 1) % 0x100;
            F_Z = (R_H == 0x00) ? 1 : 0;
            F_N = 0;
            F_H = ((R_H & 0x0F) == 0x00) ? 1 : 0;
            break;
        case 0x25: // DEC H
            R_H = (R_H + 0xFF) % 0x100;
            F_Z = (R_H == 0x00) ? 1 : 0;
            F_N = 1;
            F_H = ((R_H & 0x0F) == 0x0F) ? 1 : 0;
            break;
        case 0x26: // LD H, imm
            R_H = READ(PC++);
            break;
        case 0x27: // DAA
            D1 = R_A >> 4;
            D2 = R_A & 0x0F;
            if (F_N)
                {
                if (F_H) 
                    {
                    D2 -= 6;
                    }
                if (F_C)
                    {
                    D1 -= 6;
                    }
                if (D2 > 9)
                    {
                    D2 -= 6;
                    }
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
            F_Z = (R_A == 0) ? 1 : 0;
            F_H = 0;
            break;
        case 0x28: // JP Z, imm
            SN = READ(PC++);
			SN = (SN & 0x7F) - (SN & 0x80);
            if (F_Z)
                {
                PC += SN;
                }
            break;
        case 0x29: // ADD HL, HL
            NNNN = (R_H << 8 | R_L) + (R_H << 8 | R_L);
            F_N = 0;
            F_H = (NNNN & 0x1000) ? 1 : 0;
            F_C = (NNNN & 0xFFFF0000) ? 1 : 0;
            R_H = (NNNN & 0x0000FF00) >> 8;
            R_L = (NNNN & 0x000000FF);
            break;
        case 0x2A: // LDI A, (HL)
            R_A = READ((R_H << 8 | R_L));
            NN = ((R_H << 8 | R_L) + 1) % 0x10000;
            R_H = NN >> 8;
            R_L = NN & 0xFF;
            break;
        case 0x2B: // DEC HL
            NN = ((R_H << 8 | R_L) + 0xFFFF) % 0x10000;
            R_H = NN >> 8;
            R_L = NN & 0xFF;
            break;
        case 0x2C: // INC L
            R_L = (R_L + 1) % 0x100;
            F_Z = (R_L == 0x00) ? 1 : 0;
            F_N = 0;
            F_H = ((R_L & 0x0F) == 0x00) ? 1 : 0;
            break;
        case 0x2D: // DEC L
            R_L = (R_L + 0xFF) % 0x100;
            F_Z = (R_L == 0x00) ? 1 : 0;
            F_N = 1;
            F_H = ((R_L & 0x0F) == 0x0F) ? 1 : 0;
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
            SN = READ(PC++);
			SN = (SN & 0x7F) - (SN & 0x80);
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
            WRITE((R_H << 8 | R_L), R_A);
            NN = ((R_H << 8 | R_L) + 0xFFFF) % 0x10000;
            R_H = NN >> 8;
            R_L = NN & 0xFF;
            break;
        case 0x33: // INC SP
            SP = (SP + 1) % 0x10000;
            break;
        case 0x34: // INC (HL)
            N = (READ((R_H << 8 | R_L)) + 1) % 0x100;
            F_Z = (N == 0x00) ? 1 : 0;
            F_N = 0;
            F_H = ((N & 0x0F) == 0x00) ? 1 : 0;
            WRITE((R_H << 8 | R_L), N);
            break;
        case 0x35: // DEC (HL)
            N = (READ((R_H << 8 | R_L)) + 0xFF) % 0x100;
            F_Z = (N == 0x00) ? 1 : 0;
            F_N = 1;
            F_H = ((N & 0x0F) == 0x0F) ? 1 : 0;
            WRITE((R_H << 8 | R_L), N);
            break;
        case 0x36: // LD (HL), imm
            WRITE((R_H << 8 | R_L), READ(PC++));
            break;
        case 0x37: // SCF
            F_N = 0;
            F_H = 0;
            F_C = 1;
            break;
        case 0x38: // JP C, imm
            SN = READ(PC++);
			SN = (SN & 0x7F) - (SN & 0x80);
            if (F_C)
                {
                PC += SN;
                }
            break;
        case 0x39: // ADD HL, SP
            NNNN = (R_H << 8 | R_L) + SP;
            F_N = 0;
            F_H = (NNNN ^ (R_H << 8 | R_L) ^ SP) & 0x1000 ? 1 : 0;
            F_C = (NNNN & 0xFFFF0000) ? 1 : 0;
            R_H = (NNNN & 0x0000FF00) >> 8;
            R_L = (NNNN & 0x000000FF);
            break;
        case 0x3A: // LDD A, (HL)
            R_A = READ((R_H << 8 | R_L));
            NN = ((R_H << 8 | R_L) + 0xFFFF) % 0x10000;
            R_H = NN >> 8;
            R_L = NN & 0xFF;
            break;
        case 0x3B: // DEC SP
            SP = (SP + 0xFFFF) % 0x10000;
            break;
        case 0x3C: // INC A
            R_A = (R_A + 1) % 0x100;
            F_Z = (R_A == 0x00) ? 1 : 0;
            F_N = 0;
            F_H = ((R_A & 0x0F) == 0x00) ? 1 : 0;
            break;
        case 0x3D: // DEC A
            R_A = (R_A + 0xFF) % 0x100;
            F_Z = (R_A == 0x00) ? 1 : 0;
            F_N = 1;
            F_H = ((R_A & 0x0F) == 0x0F) ? 1 : 0;
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
            R_B = READ((R_H << 8 | R_L));
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
            R_C = READ((R_H << 8 | R_L));
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
            R_D = READ((R_H << 8 | R_L));
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
            R_E = READ((R_H << 8 | R_L));
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
            R_H = READ((R_H << 8 | R_L));
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
            R_L = READ((R_H << 8 | R_L));
            break;
        case 0x6F: // LD L, A
            R_L = R_A;
            break;
        case 0x70: // LD (HL), B
            WRITE((R_H << 8 | R_L), R_B);
            break;
        case 0x71: // LD (HL), C
            WRITE((R_H << 8 | R_L), R_C);
            break;
        case 0x72: // LD (HL), D
            WRITE((R_H << 8 | R_L), R_D);
            break;
        case 0x73: // LD (HL), E
            WRITE((R_H << 8 | R_L), R_E);
            break;
        case 0x74: // LD (HL), H
            WRITE((R_H << 8 | R_L), R_H);
            break;
        case 0x75: // LD (HL), L
            WRITE((R_H << 8 | R_L), R_L);
            break;
        case 0x76: // HALT
            gb_halt = 1;
            break;
        case 0x77: // LD (HL), A
            WRITE((R_H << 8 | R_L), R_A);
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
            R_A = READ((R_H << 8 | R_L));
            break;
        case 0x7F: // LD A, A
            break;
        case 0x80: // ADD A, B
            NN = R_A + R_B;
            F_Z = ((NN & 0xFF) == 0x00) ? 1 : 0;
            F_N = 0;
            F_H = (R_A ^ R_B ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            R_A = NN & 0xFF;
            break;
        case 0x81: // ADD A, C
            NN = R_A + R_C;
            F_Z = ((NN & 0xFF) == 0x00) ? 1 : 0;
            F_N = 0;
            F_H = (R_A ^ R_C ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            R_A = NN & 0xFF;
            break;
        case 0x82: // ADD A, D
            NN = R_A + R_D;
            F_Z = ((NN & 0xFF) == 0x00) ? 1 : 0;
            F_N = 0;
            F_H = (R_A ^ R_D ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            R_A = NN & 0xFF;
            break;
        case 0x83: // ADD A, E
            NN = R_A + R_E;
            F_Z = ((NN & 0xFF) == 0x00) ? 1 : 0;
            F_N = 0;
            F_H = (R_A ^ R_E ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            R_A = NN & 0xFF;
            break;
        case 0x84: // ADD A, H
            NN = R_A + R_H;
            F_Z = ((NN & 0xFF) == 0x00) ? 1 : 0;
            F_N = 0;
            F_H = (R_A ^ R_H ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            R_A = NN & 0xFF;
            break;
        case 0x85: // ADD A, L
            NN = R_A + R_L;
            F_Z = ((NN & 0xFF) == 0x00) ? 1 : 0;
            F_N = 0;
            F_H = (R_A ^ R_L ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            R_A = NN & 0xFF;
            break;
        case 0x86: // ADD A, (HL)
            N = READ((R_H << 8 | R_L));
            NN = R_A + N;
            F_Z = ((NN & 0xFF) == 0x00) ? 1 : 0;
            F_N = 0;
            F_H = (R_A ^ N ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            R_A = NN & 0xFF;
            break;
        case 0x87: // ADD A, A
            NN = R_A + R_A;
            F_Z = ((NN & 0xFF) == 0x00) ? 1 : 0;
            F_N = 0;
            F_H = NN & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            R_A = NN & 0xFF;
            break;
        case 0x88: // ADC A, B
            N = R_B;
            NN = R_A + N + F_C;
            F_Z = ((NN & 0xFF) == 0x00) ? 1 : 0;
            F_N = 0;
            F_H = (R_A ^ N ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            R_A = NN & 0xFF;
            break;
        case 0x89: // ADC A, C
            N = R_C;
            NN = R_A + N + F_C;
            F_Z = ((NN & 0xFF) == 0x00) ? 1 : 0;
            F_N = 0;
            F_H = (R_A ^ N ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            R_A = NN & 0xFF;
            break;
        case 0x8A: // ADC A, D
            N = R_D;
            NN = R_A + N + F_C;
            F_Z = ((NN & 0xFF) == 0x00) ? 1 : 0;
            F_N = 0;
            F_H = (R_A ^ N ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            R_A = NN & 0xFF;
            break;
        case 0x8B: // ADC A, E
            N = R_E;
            NN = R_A + N + F_C;
            F_Z = ((NN & 0xFF) == 0x00) ? 1 : 0;
            F_N = 0;
            F_H = (R_A ^ N ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            R_A = NN & 0xFF;
            break;
        case 0x8C: // ADC A, H
            N = R_H;
            NN = R_A + N + F_C;
            F_Z = ((NN & 0xFF) == 0x00) ? 1 : 0;
            F_N = 0;
            F_H = (R_A ^ N ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            R_A = NN & 0xFF;
            break;
        case 0x8D: // ADC A, L
            N = R_L;
            NN = R_A + N + F_C;
            F_Z = ((NN & 0xFF) == 0x00) ? 1 : 0;
            F_N = 0;
            F_H = (R_A ^ N ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            R_A = NN & 0xFF;
            break;
        case 0x8E: // ADC A, (HL)
            N = READ((R_H << 8 | R_L));
            NN = R_A + N + F_C;
            F_Z = ((NN & 0xFF) == 0x00) ? 1 : 0;
            F_N = 0;
            F_H = (R_A ^ N ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            R_A = NN & 0xFF;
            break;
        case 0x8F: // ADC A, A
            N = R_A;
            NN = R_A + N + F_C;
            F_Z = ((NN & 0xFF) == 0x00) ? 1 : 0;
            F_N = 0;
            F_H = (R_A ^ N ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            R_A = NN & 0xFF;
            break;
        case 0x90: // SUB B
            NN = R_A - R_B + 0x10000;
            F_Z = ((NN & 0xFF) == 0x00) ? 1 : 0;
            F_N = 1;
            F_H = (R_A ^ R_B ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            R_A = NN & 0xFF;
            break;
        case 0x91: // SUB C
            NN = R_A - R_C + 0x10000;
            F_Z = ((NN & 0xFF) == 0x00) ? 1 : 0;
            F_N = 1;
            F_H = (R_A ^ R_C ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            R_A = NN & 0xFF;
            break;
        case 0x92: // SUB D
            NN = R_A - R_D + 0x10000;
            F_Z = ((NN & 0xFF) == 0x00) ? 1 : 0;
            F_N = 1;
            F_H = (R_A ^ R_D ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            R_A = NN & 0xFF;
            break;
        case 0x93: // SUB E
            NN = R_A - R_E + 0x10000;
            F_Z = ((NN & 0xFF) == 0x00) ? 1 : 0;
            F_N = 1;
            F_H = (R_A ^ R_E ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            R_A = NN & 0xFF;
            break;
        case 0x94: // SUB H
            NN = R_A - R_H + 0x10000;
            F_Z = ((NN & 0xFF) == 0x00) ? 1 : 0;
            F_N = 1;
            F_H = (R_A ^ R_H ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            R_A = NN & 0xFF;
            break;
        case 0x95: // SUB L
            NN = R_A - R_L + 0x10000;
            F_Z = ((NN & 0xFF) == 0x00) ? 1 : 0;
            F_N = 1;
            F_H = (R_A ^ R_L ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            R_A = NN & 0xFF;
            break;
        case 0x96: // SUB (HL)
            N = READ((R_H << 8 | R_L));
            NN = R_A - N + 0x10000;
            F_Z = ((NN & 0xFF) == 0x00) ? 1 : 0;
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
            NN = R_A - N - F_C + 0x10000;
            F_Z = ((NN & 0xFF) == 0x00) ? 1 : 0;
            F_N = 1;
            F_H = (R_A ^ N ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            R_A = NN & 0xFF;
            break;
        case 0x99: // SBC A, C
            N = R_C;
            NN = R_A - N - F_C + 0x10000;
            F_Z = ((NN & 0xFF) == 0x00) ? 1 : 0;
            F_N = 1;
            F_H = (R_A ^ N ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            R_A = NN & 0xFF;
            break;
        case 0x9A: // SBC A, D
            N = R_D;
            NN = R_A - N - F_C + 0x10000;
            F_Z = ((NN & 0xFF) == 0x00) ? 1 : 0;
            F_N = 1;
            F_H = (R_A ^ N ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            R_A = NN & 0xFF;
            break;
        case 0x9B: // SBC A, E
            N = R_E;
            NN = R_A - N - F_C + 0x10000;
            F_Z = ((NN & 0xFF) == 0x00) ? 1 : 0;
            F_N = 1;
            F_H = (R_A ^ N ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            R_A = NN & 0xFF;
            break;
        case 0x9C: // SBC A, H
            N = R_H;
            NN = R_A - N - F_C + 0x10000;
            F_Z = ((NN & 0xFF) == 0x00) ? 1 : 0;
            F_N = 1;
            F_H = (R_A ^ N ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            R_A = NN & 0xFF;
            break;
        case 0x9D: // SBC A, L
            N = R_L;
            NN = R_A - N - F_C + 0x10000;
            F_Z = ((NN & 0xFF) == 0x00) ? 1 : 0;
            F_N = 1;
            F_H = (R_A ^ N ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            R_A = NN & 0xFF;
            break;
        case 0x9E: // SBC A, (HL)
            N = READ((R_H << 8 | R_L));
            NN = R_A - N - F_C + 0x10000;
            F_Z = ((NN & 0xFF) == 0x00) ? 1 : 0;
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
            F_Z = (R_A == 0x00) ? 1 : 0;
            F_N = 0;
            F_H = 1;
            F_C = 0;
            break;
        case 0xA1: // AND C
            R_A = R_A & R_C;
            F_Z = (R_A == 0x00) ? 1 : 0;
            F_N = 0;
            F_H = 1;
            F_C = 0;
            break;
        case 0xA2: // AND D
            R_A = R_A & R_D;
            F_Z = (R_A == 0x00) ? 1 : 0;
            F_N = 0;
            F_H = 1;
            F_C = 0;
            break;
        case 0xA3: // AND E
            R_A = R_A & R_E;
            F_Z = (R_A == 0x00) ? 1 : 0;
            F_N = 0;
            F_H = 1;
            F_C = 0;
            break;
        case 0xA4: // AND H
            R_A = R_A & R_H;
            F_Z = (R_A == 0x00) ? 1 : 0;
            F_N = 0;
            F_H = 1;
            F_C = 0;
            break;
        case 0xA5: // AND L
            R_A = R_A & R_L;
            F_Z = (R_A == 0x00) ? 1 : 0;
            F_N = 0;
            F_H = 1;
            F_C = 0;
            break;
        case 0xA6: // AND (HL)
            R_A = R_A & READ((R_H << 8 | R_L));
            F_Z = (R_A == 0x00) ? 1 : 0;
            F_N = 0;
            F_H = 1;
            F_C = 0;
            break;
        case 0xA7: // AND A
            F_Z = (R_A == 0x00) ? 1 : 0;
            F_N = 0;
            F_H = 1;
            F_C = 0;
            break;
        case 0xA8: // XOR B
            R_A = R_A ^ R_B;
            F_Z = (R_A == 0x00) ? 1 : 0;
            F_N = 0;
            F_H = 0;
            F_C = 0;
            break;
        case 0xA9: // XOR C
            R_A = R_A ^ R_C;
            F_Z = (R_A == 0x00) ? 1 : 0;
            F_N = 0;
            F_H = 0;
            F_C = 0;
            break;
        case 0xAA: // XOR D
            R_A = R_A ^ R_D;
            F_Z = (R_A == 0x00) ? 1 : 0;
            F_N = 0;
            F_H = 0;
            F_C = 0;
            break;
        case 0xAB: // XOR E
            R_A = R_A ^ R_E;
            F_Z = (R_A == 0x00) ? 1 : 0;
            F_N = 0;
            F_H = 0;
            F_C = 0;
            break;
        case 0xAC: // XOR H
            R_A = R_A ^ R_H;
            F_Z = (R_A == 0x00) ? 1 : 0;
            F_N = 0;
            F_H = 0;
            F_C = 0;
            break;
        case 0xAD: // XOR L
            R_A = R_A ^ R_L;
            F_Z = (R_A == 0x00) ? 1 : 0;
            F_N = 0;
            F_H = 0;
            F_C = 0;
            break;
        case 0xAE: // XOR (HL)
            R_A = R_A ^ READ((R_H << 8 | R_L));
            F_Z = (R_A == 0x00) ? 1 : 0;
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
            F_Z = (R_A == 0x00) ? 1 : 0;
            F_N = 0;
            F_H = 0;
            F_C = 0;
            break;
        case 0xB1: // OR C
            R_A = R_A | R_C;
            F_Z = (R_A == 0x00) ? 1 : 0;
            F_N = 0;
            F_H = 0;
            F_C = 0;
            break;
        case 0xB2: // OR D
            R_A = R_A | R_D;
            F_Z = (R_A == 0x00) ? 1 : 0;
            F_N = 0;
            F_H = 0;
            F_C = 0;
            break;
        case 0xB3: // OR E
            R_A = R_A | R_E;
            F_Z = (R_A == 0x00) ? 1 : 0;
            F_N = 0;
            F_H = 0;
            F_C = 0;
            break;
        case 0xB4: // OR H
            R_A = R_A | R_H;
            F_Z = (R_A == 0x00) ? 1 : 0;
            F_N = 0;
            F_H = 0;
            F_C = 0;
            break;
        case 0xB5: // OR L
            R_A = R_A | R_L;
            F_Z = (R_A == 0x00) ? 1 : 0;
            F_N = 0;
            F_H = 0;
            F_C = 0;
            break;
        case 0xB6: // OR (HL)
            R_A = R_A | READ((R_H << 8 | R_L));
            F_Z = (R_A == 0x00) ? 1 : 0;
            F_N = 0;
            F_H = 0;
            F_C = 0;
            break;
        case 0xB7: // OR A
            F_Z = (R_A == 0x00) ? 1 : 0;
            F_N = 0;
            F_H = 0;
            F_C = 0;
            break;
        case 0xB8: // CP B
            NN = R_A - R_B + 0x10000;
            F_Z = ((NN & 0xFF) == 0x00) ? 1 : 0;
            F_N = 1;
            F_H = (R_A ^ R_B ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            break;
        case 0xB9: // CP C
            NN = R_A - R_C + 0x10000;
            F_Z = ((NN & 0xFF) == 0x00) ? 1 : 0;
            F_N = 1;
            F_H = (R_A ^ R_C ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            break;
        case 0xBA: // CP D
            NN = R_A - R_D + 0x10000;
            F_Z = ((NN & 0xFF) == 0x00) ? 1 : 0;
            F_N = 1;
            F_H = (R_A ^ R_D ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            break;
        case 0xBB: // CP E
            NN = R_A - R_E + 0x10000;
            F_Z = ((NN & 0xFF) == 0x00) ? 1 : 0;
            F_N = 1;
            F_H = (R_A ^ R_E ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            break;
        case 0xBC: // CP H
            NN = R_A - R_H + 0x10000;
            F_Z = ((NN & 0xFF) == 0x00) ? 1 : 0;
            F_N = 1;
            F_H = (R_A ^ R_H ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            break;
        case 0xBD: // CP L
            NN = R_A - R_L + 0x10000;
            F_Z = ((NN & 0xFF) == 0x00) ? 1 : 0;
            F_N = 1;
            F_H = (R_A ^ R_L ^ NN) & 0x10 ? 1 : 0;
            F_C = (NN & 0xFF00) ? 1 : 0;
            break;
        case 0xBE: // CP (HL)
            N = READ((R_H << 8 | R_L));
            NN = R_A - N + 0x10000;
            F_Z = ((NN & 0xFF) == 0x00) ? 1 : 0;
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
            F_Z = ((NN & 0xFF) == 0x00) ? 1 : 0;
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
            NN = R_A - N + 0x10000;
            F_Z = ((NN & 0xFF) == 0x00) ? 1 : 0;
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
            NN = R_A - N - F_C + 0x10000;
            F_Z = ((NN & 0xFF) == 0x00) ? 1 : 0;
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
            F_Z = (R_A == 0x00) ? 1 : 0;
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
            SN = READ(PC++);
			SN = (SN & 0x7F) - (SN & 0x80);
            NN = (SP + SN + 0x10000) % 0x10000;
            if (SN >= 0)
                {
                F_Z = 0;
                F_N = 0;
                F_H = ((SP ^ SN ^ NN) & 0x1000) ? 1 : 0;
                F_C = (SP > NN) ? 1 : 0;
                }
            else
                {
                F_Z = 0;
                F_N = 0;
                F_H = ((SP ^ SN ^ NN) & 0x1000) ? 1 : 0;
                F_C = (SP < NN) ? 1 : 0;
                }
            SP = NN;
            break;
        case 0xE9: // JP (HL)
            PC = (R_H << 8 | R_L);
            break;
        case 0xEA: // LD (imm), A
            WRITE(READ(PC++) | READ(PC++) << 8, R_A);
            break;
        case 0xEB: // illegal
            break;
        case 0xEC: // illegal
            break;
        case 0xED: // illegal
            break;
        case 0xEE: // XOR imm
            R_A = R_A ^ READ(PC++);
            F_Z = (R_A == 0x00) ? 1 : 0;
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
            F_Z = (R_A == 0x00) ? 1 : 0;
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
            SN = READ(PC++);
			SN = (SN & 0x7F) - (SN & 0x80);
            NN = (SP + SN + 0x10000) % 0x10000;
            if (SN >= 0)
                {
                F_Z = 0;
                F_N = 0;
                F_H = ((SP ^ SN ^ NN) & 0x1000) ? 1 : 0;
                F_C = (SP > NN) ? 1 : 0;
                }
            else
                {
                F_Z = 0;
                F_N = 0;
                F_H = ((SP ^ SN ^ NN) & 0x1000) ? 1 : 0;
                F_C = (SP < NN) ? 1 : 0;
                }
            R_H = (NN & 0xFF00) >> 8;
            R_L = (NN & 0x00FF);
            break;
        case 0xF9: // LD SP, HL
            SP = (R_H << 8 | R_L);
            break;
        case 0xFA: // LD A, (imm)
            R_A = READ(READ(PC++) | READ(PC++) << 8);
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
            NN = R_A - N + 0x10000;
            F_Z = ((NN & 0xFF) == 0x00) ? 1 : 0;
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
                // copy 0x10 bytes each hblank
                for (var i = 0; i < 0x10; i++)
                    WRITE(R_HDMAD + (R_HDMA * 0x10) + i, READ(R_HDMAS + (R_HDMA * 0x10) + i));
                R_HDMA--;
                lcd_count += 8 * (cgb_double + 1);
                inst_cycles += 8 * (cgb_double + 1);
                }
            }
        }
    // OAM access
    else if (lcd_mode == LCD_HBLANK && lcd_count >= LCD_MODE_2_CYCLES * (cgb_double + 1))
        {
        lcd_mode = LCD_SEARCH_OAM;
        if (R_STAT & STAT_MODE_2_INTR)
            R_IF |= LCDC_INTR;
        }        
    // Update LCD
    else if (lcd_mode == LCD_SEARCH_OAM && lcd_count >= LCD_MODE_3_CYCLES * (cgb_double + 1))
        {
        lcd_mode = LCD_TRANSFER;
        LCDDrawLine();
        }
        
    // DIV register timing
    div_count += inst_cycles;
    if (div_count > DIV_CYCLES)
        {
        R_DIV = (R_DIV + 1) % 0x100;
        div_count -= DIV_CYCLES;
        }
    
    // TIMA register timing
    if (tac_enable)
        {
        tima_count += inst_cycles;
        if (tima_count > TAC_CYCLES[tac_rate])
            {
            tima_count -= TAC_CYCLES[tac_rate];
            R_TIMA = (R_TIMA + 1) % 0x100;
            if (R_TIMA == 0)
                {
                R_IF |= TIMER_INTR;
                R_TIMA = R_TMA;
                }
            }
        }
    }



// CB helper variables
var CBOP;
var R;
var B;
var D;
var val;
var writeback;

// bit-level operations
function ExecuteCB()
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
        case 6: val = READ((R_H << 8 | R_L)); break;
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
                        val |= CBOP ? (F_C << 7) : ((N << 7) & 0x80);
                        F_Z = (val == 0x00) ? 1 : 0;
                        F_N = 0;
                        F_H = 0;
                        F_C = (N & 0x01) ? 1 : 0;
                        }
                    else    // RLC R / RL R
                        {
                        N = val;
                        val = (val << 1) & 0xFF;
                        val |= CBOP ? F_C : (N >> 7);
                        F_Z = (val == 0x00) ? 1 : 0;
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
                        F_Z = (val == 0x00) ? 1 : 0;
                        F_N = 0;
                        F_H = 0;
                        }
                    else    // SLA R
                        {
                        F_C = (val >> 7);
                        val = (val << 1) & 0xFF;
                        F_Z = (val == 0x00) ? 1 : 0;
                        F_N = 0;
                        F_H = 0;
                        }
                    break;
                case 0x3:
                    if (D) // SRL R
                        {
                        F_C = val & 0x01;
                        val = val >> 1;
                        F_Z = (val == 0x00) ? 1 : 0;
                        F_N = 0;
                        F_H = 0;
                        }
                    else    // SWAP R
                        {
                        N = (val >> 4) & 0x0F;
                        N |= (val << 4) & 0xF0;
                        val = N;
                        F_Z = (val == 0) ? 1 : 0;
                        F_N = 0;
                        F_H = 0;
                        F_C = 0;
                        }
                    break;
                }
            break;
        case 0x1: // BIT B, R
            F_Z = ((val >> B) & 0x1) ? 0 : 1;
            F_N = 0;
            F_H = 1;
            writeback = 0;
            break;
        case 0x2: // RES B, R
            val &= (0xFE << B) | (0xFF >> (8-B));
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
            case 6: WRITE((R_H << 8 | R_L), val); break;
            case 7: R_A = val; break;
            }
        }
    }




/*

// INC
// DEC

// LD
// LD REG, REG
// LD REG, imm
// LD (REG), REG

OP[0x00] = function() {};//"NOP",              
OP[0x01] = //"LD BC, imm",       
OP[0x02] = //"LD (BC), A",       
OP[0x03] = //"INC BC",
OP[0x04] = //"INC B",            
OP[0x05] = //"DEC B",            
OP[0x06] = //"LD B, imm",        
OP[0x07] = //"RLCA",
OP[0x08] = //"LD (imm), SP",     
OP[0x09] = //"ADD HL, BC",       
OP[0x0A] = //"LD A, (BC)",       
OP[0x0B] = //"DEC BC",
OP[0x0C] = //"INC C",            
OP[0x0D] = //"DEC C",            
OP[0x0E] = //"LD C, imm",        
OP[0x0F] = //"RRCA",
// 0x1
OP[0x10] = //"STOP",             
OP[0x11] = //"LD DE, imm",       
OP[0x12] = //"LD (DE), A",       
OP[0x13] = //"INC DE",
OP[0x14] = //"INC D",            
OP[0x15] = //"DEC D",            
OP[0x16] = //"LD D, imm",        
OP[0x17] = //"RLA",
OP[0x18] = //"JR imm",           
OP[0x19] = //"ADD HL, DE",       
OP[0x1A] = //"LD A, (DE)",       
OP[0x1B] = //"DEC DE",
OP[0x1C] = //"INC E",            
OP[0x1D] = //"DEC E",            
OP[0x1E] = //"LD E, imm",        
OP[0x1F] = //"RRA",
// 0x2
OP[0x20] = //"JP NZ, imm",       
OP[0x21] = //"LD HL, imm",       
OP[0x22] = //"LDI (HL), A",      
OP[0x23] = //"INC HL",
OP[0x24] = //"INC H",            
OP[0x25] = //"DEC H",            
OP[0x26] = //"LD H, imm",        
OP[0x27] = //"DAA",
OP[0x28] = //"JP Z, imm",        
OP[0x29] = //"ADD HL, HL",       
OP[0x2A] = //"LDI A, (HL)",      
OP[0x2B] = //"DEC HL",
OP[0x2C] = //"INC L",            
OP[0x2D] = //"DEC L",            
OP[0x2E] = //"LD L, imm",        
OP[0x2F] = //"CPL",
// 0x3
OP[0x30] = //"JP NC, imm",       
OP[0x31] = //"LD SP, imm",       
OP[0x32] = //"LDD (HL), A",      
OP[0x33] = //"INC SP",
OP[0x34] = //"INC (HL)",         
OP[0x35] = //"DEC (HL)",         
OP[0x36] = //"LD (HL), imm",     
OP[0x37] = //"SCF",
OP[0x38] = //"JP C, imm",        
OP[0x39] = //"ADD HL, SP",       
OP[0x3A] = //"LDD A, (HL)",      
OP[0x3B] = //"DEC SP",
OP[0x3C] = //"INC A",            
OP[0x3D] = //"DEC A",            
OP[0x3E] = //"LD A, imm",        
OP[0x3F] = //"CCF",
// 0x4
OP[0x40] = MakeLD("B","B"); //"LD B, B",          
OP[0x41] = MakeLD("B","C"); //"LD B, C",          
OP[0x42] = MakeLD("B","D"); //"LD B, D",          
OP[0x43] = MakeLD("B","E"); //"LD B, E",
OP[0x44] = MakeLD("B","H"); //"LD B, H",          
OP[0x45] = MakeLD("B","L"); //"LD B, L",          
OP[0x46] = MakeLD("B","(HL)"); //"LD B, (HL)",       
OP[0x47] = MakeLD("B","A"); //"LD B, A",
OP[0x48] = MakeLD("C","B"); //"LD C, B",          
OP[0x49] = MakeLD("C","C"); //"LD C, C",          
OP[0x4A] = MakeLD("C","D"); //"LD C, D",          
OP[0x4B] = MakeLD("C","E"); //"LD C, E",
OP[0x4C] = MakeLD("C","H"); //"LD C, H",          
OP[0x4D] = MakeLD("C","L"); //"LD C, L",          
OP[0x4E] = MakeLD("C","(HL)"); //"LD C, (HL)",       
OP[0x4F] = MakeLD("C","A"); //"LD C, A",
// 0x5
OP[0x50] = MakeLD("D","B"); //"LD D, B",          
OP[0x51] = MakeLD("D","C"); //"LD D, C",          
OP[0x52] = MakeLD("D","D"); //"LD D, D",          
OP[0x53] = MakeLD("D","E"); //"LD D, E",
OP[0x54] = MakeLD("D","H"); //"LD D, H",          
OP[0x55] = MakeLD("D","L"); //"LD D, L",          
OP[0x56] = MakeLD("D","(HL)"); //"LD D, (HL)",       
OP[0x57] = MakeLD("D","A"); //"LD D, A",
OP[0x58] = MakeLD("E","B"); //"LD E, B",          
OP[0x59] = MakeLD("E","C"); //"LD E, C",          
OP[0x5A] = MakeLD("E","D"); //"LD E, D",          
OP[0x5B] = MakeLD("E","E"); //"LD E, E",
OP[0x5C] = MakeLD("E","H"); //"LD E, H",          
OP[0x5D] = MakeLD("E","L"); //"LD E, L",          
OP[0x5E] = MakeLD("E","(HL)"); //"LD E, (HL)",       
OP[0x5F] = MakeLD("E","A"); //"LD E, A",
// 0x6
OP[0x60] = MakeLD("H","B"); //"LD H, B",          
OP[0x61] = MakeLD("H","C"); //"LD H, C",          
OP[0x62] = MakeLD("H","D"); //"LD H, D",          
OP[0x63] = MakeLD("H","E"); //"LD H, E",
OP[0x64] = MakeLD("H","H"); //"LD H, H",          
OP[0x65] = MakeLD("H","L"); //"LD H, L",          
OP[0x66] = MakeLD("H","(HL)"); //"LD H, (HL)",       
OP[0x67] = MakeLD("H","A"); //"LD H, A",
OP[0x68] = MakeLD("L","B"); //"LD L, B",          
OP[0x69] = MakeLD("L","C"); //"LD L, C",          
OP[0x6A] = MakeLD("L","D"); //"LD L, D",          
OP[0x6B] = MakeLD("L","E"); //"LD L, E",
OP[0x6C] = MakeLD("L","H"); //"LD L, H",          
OP[0x6D] = MakeLD("L","L"); //"LD L, L",          
OP[0x6E] = MakeLD("L","(HL)"); //"LD L, (HL)",       
OP[0x6F] = MakeLD("L","A"); //"LD L, A",
// 0x7
OP[0x70] = MakeLD("(HL)","B"); //"LD (HL), B",       
OP[0x71] = MakeLD("(HL)","C"); //"LD (HL), C",       
OP[0x72] = MakeLD("(HL)","D"); //"LD (HL), D",       
OP[0x73] = MakeLD("(HL)","E"); //"LD (HL), E",
OP[0x74] = MakeLD("(HL)","H"); //"LD (HL), H",       
OP[0x75] = MakeLD("(HL)","L"); //"LD (HL), L",       
OP[0x76] = //"HALT",             
OP[0x77] = MakeLD("(HL)","A"); //"LD (HL), A",
OP[0x78] = MakeLD("A","B"); //"LD A, B",          
OP[0x79] = MakeLD("A","C"); //"LD A, C",          
OP[0x7A] = MakeLD("A","D"); //"LD A, D",          
OP[0x7B] = MakeLD("A","E"); //"LD A, E",
OP[0x7C] = MakeLD("A","H"); //"LD A, H",          
OP[0x7D] = MakeLD("A","L"); //"LD A, L",          
OP[0x7E] = MakeLD("A","(HL)"); //"LD A, (HL)",       
OP[0x7F] = MakeLD("A","A"); //"LD A, A",
// 0x8
OP[0x80] = //"ADD A, B",         
OP[0x81] = //"ADD A, C",         
OP[0x82] = //"ADD A, D",         
OP[0x83] = //"ADD A, E",
OP[0x84] = //"ADD A, H",         
OP[0x85] = //"ADD A, L",         
OP[0x86] = //"ADD A, (HL)",      
OP[0x87] = //"ADD A, A",
OP[0x88] = //"ADC A, B",         
OP[0x89] = //"ADC A, C",         
OP[0x8A] = //"ADC A, D",         
OP[0x8B] = //"ADC A, E",
OP[0x8C] = //"ADC A, H",         
OP[0x8D] = //"ADC A, L",         
OP[0x8E] = //"ADC A, (HL)",      
OP[0x8F] = //"ADC A, A",
// 0x9
OP[0x90] = //"SUB B",            
OP[0x91] = //"SUB C",            
OP[0x92] = //"SUB D",            
OP[0x93] = //"SUB E",
OP[0x94] = //"SUB H",            
OP[0x95] = //"SUB L",            
OP[0x96] = //"SUB (HL)",         
OP[0x97] = //"SUB A",
OP[0x98] = //"SBC A, B",         
OP[0x99] = //"SBC A, C",         
OP[0x9A] = //"SBC A, D",         
OP[0x9B] = //"SBC A, E",
OP[0x9C] = //"SBC A, H",         
OP[0x9D] = //"SBC A, L",         
OP[0x9E] = //"SBC A, (HL)",      
OP[0x9F] = //"SBC A, A",
// 0xA
OP[0xA0] = //"AND B",            
OP[0xA1] = //"AND C",            
OP[0xA2] = //"AND D",            
OP[0xA3] = //"AND E",
OP[0xA4] = //"AND H",            
OP[0xA5] = //"AND L",            
OP[0xA6] = //"AND (HL)",         
OP[0xA7] = //"AND A",
OP[0xA8] = //"XOR B",            
OP[0xA9] = //"XOR C",            
OP[0xAA] = //"XOR D",            
OP[0xAB] = //"XOR E",
OP[0xAC] = //"XOR H",            
OP[0xAD] = //"XOR L",            
OP[0xAE] = //"XOR (HL)",         
OP[0xAF] = //"XOR A",
// 0xB
OP[0xB0] = //"OR B",             
OP[0xB1] = //"OR C",             
OP[0xB2] = //"OR D",             
OP[0xB3] = //"OR E",
OP[0xB4] = //"OR H",             
OP[0xB5] = //"OR L",             
OP[0xB6] = //"OR (HL)",          
OP[0xB7] = //"OR A",
OP[0xB8] = //"CP B",             
OP[0xB9] = //"CP C",             
OP[0xBA] = //"CP D",             
OP[0xBB] = //"CP E",
OP[0xBC] = //"CP H",             
OP[0xBD] = //"CP L",             
OP[0xBE] = //"CP (HL)",          
OP[0xBF] = //"CP A",
// 0xC
OP[0xC0] = //"RET NZ",           
OP[0xC1] = //"POP BC",           
OP[0xC2] = //"JP NZ, imm",       
OP[0xC3] = //"JP imm",
OP[0xC4] = //"CALL NZ, imm",     
OP[0xC5] = //"PUSH BC",          
OP[0xC6] = //"ADD A, imm",       
OP[0xC7] = //"RST 0x0000",
OP[0xC8] = //"RET Z",            
OP[0xC9] = //"RET",              
OP[0xCA] = //"JP Z, imm",        
OP[0xCB] = //"CB INST",
OP[0xCC] = //"CALL Z, imm",      
OP[0xCD] = //"CALL imm",         
OP[0xCE] = //"ADC A, imm",       
OP[0xCF] = //"RST 0x0008",
// 0xD
OP[0xD0] = //"RET NC",           
OP[0xD1] = //"POP DE",           
OP[0xD2] = //"JP NC, imm",       
OP[0xD3] = //"illegal",
OP[0xD4] = //"CALL NC, imm",     
OP[0xD5] = //"PUSH DE",          
OP[0xD6] = //"SUB imm",          
OP[0xD7] = //"RST 0x0010",
OP[0xD8] = //"RET C",            
OP[0xD9] = //"RETI",             
OP[0xDA] = //"JP C, imm",        
OP[0xDB] = //"illegal",
OP[0xDC] = //"CALL C, imm",      
OP[0xDD] = //"illegal",          
OP[0xDE] = //"SBC A, imm",       
OP[0xDF] = //"RST 0x0018",
// 0xE
OP[0xE0] = //"LD (0xFF00+imm), A",
OP[0xE1] = //"POP HL",          
OP[0xE2] = //"LD (C), A",        
OP[0xE3] = //"illegal",
OP[0xE4] = //"illegal",          
OP[0xE5] = //"PUSH HL",          
OP[0xE6] = //"AND imm",          
OP[0xE7] = //"RST 0x0020",
OP[0xE8] = //"ADD SP, imm",      
OP[0xE9] = //"JP (HL)",          
OP[0xEA] = //"LD (imm), A",      
OP[0xEB] = //"illegal",
OP[0xEC] = //"illegal",          
OP[0xED] = //"illegal",          
OP[0xEE] = //"XOR imm",          
OP[0xEF] = //"RST 0x0028",
// 0xF
OP[0xF0] = //"LD A, (0xFF00+imm)",
OP[0xF1] = //"POP AF",          
OP[0xF2] = //"LD A, (C)",        
OP[0xF3] = //"DI",
OP[0xF4] = //"illegal",          
OP[0xF5] = //"PUSH AF",          
OP[0xF6] = //"OR imm",           
OP[0xF7] = //"RST 0x0030",
OP[0xF8] = //"LD HL, SP+/-imm",  
OP[0xF9] = //"LD SP, HL",        
OP[0xFA] = //"LD A, (imm)",      
OP[0xFB] = //"EI",
OP[0xFC] = //"illegal",          
OP[0xFD] = //"illegal",          
OP[0xFE] = //"CP imm",           
OP[0xFF] = //"RST 0x0038"
*/


/*

// Instruction Mnuemonics
OP_MN[] = [
	// 0x0
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
    ];


*/

/**********
 screen.c
 --------------------
 GameBoy emulator
 Credits: Greg Tourville
*/

#include <string.h>
#include "gameboy.h"


// VIDEO ////////////////////////////
u8 WY, WYC, WX;
u8 X, Y;

extern u8 R_WX;
extern u8 R_WY;
extern u8 R_SCY;
extern u8 R_SCX;
extern u8 R_LY;
extern u8 R_LYC;
extern u8 R_LCDC;

u8 BX;
u8 BY;
u8 SX[LCD_WIDTH];
void LCDDrawLine()
    {
    u16 bg_line;
    u16 win_line;
    u16 tile;
    u8  t1, t2, c;
    u8  count = 0;

    if (R_LCDC & LCDC_BG_ENABLE)
        {
        BY = R_LY + R_SCY;
        bg_line = (R_LCDC & LCDC_BG_MAP) ? VRAM_BMAP_2 : VRAM_BMAP_1;
        bg_line += (BY >> 3) * 0x20;
        }
    else
        bg_line = 0;

    if (R_LCDC & LCDC_WINDOW_ENABLE && R_LY >= WY && R_WX <= 166)
        {
        win_line = (R_LCDC & LCDC_WINDOW_MAP) ? VRAM_BMAP_2 : VRAM_BMAP_1;
        win_line += (WYC >> 3) * 0x20;
        }
    else
        win_line = 0;

    memset(SX, 0xFF, LCD_WIDTH);

    // draw background
    if (bg_line)
        {
        u8 px, py, idx;

        X = LCD_WIDTH - 1;
        BX = X + R_SCX;

        // lookup tile index
        py = (BY & 0x07);
        px = 7 - (BX & 0x07);
        idx = VRAM[bg_line + (BX >> 3)];
        if (R_LCDC & LCDC_TILE_SELECT)
            tile = VRAM_TILES_1 + idx * 0x10;
        else
            tile = VRAM_TILES_2 + ((idx + 0x80) % 0x100) * 0x10;
        tile += 2*py;

        // fetch first tile
        t1 = VRAM[tile] >> px;
        t2 = VRAM[tile+1] >> px;
        for (; X != 0xFF; X--)
            {
            SX[X] = 0xFE;
            if (px == 8)
                {
                // fetch next tile
                px = 0;
                BX = X + R_SCX;
                idx = VRAM[bg_line + (BX >> 3)];
                if (R_LCDC & LCDC_TILE_SELECT)
                    tile = VRAM_TILES_1 + idx * 0x10;
                else
                    tile = VRAM_TILES_2 + ((idx + 0x80) % 0x100) * 0x10;
                tile += 2*py;
                t1 = VRAM[tile];
                t2 = VRAM[tile+1];
                }
            // copy background
            c = (t1 & 0x1) | ((t2 & 0x1) << 1);
            if (R_LY < LCD_HEIGHT && X < LCD_WIDTH)
                {
                gb_fb[R_LY][X] = c;// BGP[c];
                }
            t1 = t1 >> 1;
            t2 = t2 >> 1;
            px++;
            }
        }

    // draw window
    if (win_line)
        {
        u8 px, py, idx;
        u8 end;

        X = LCD_WIDTH - 1;
        WX = X - R_WX + 7;

        // look up tile
        py = WYC & 0x07;
        px = 7 - (WX & 0x07);
        idx = VRAM[win_line + (WX >> 3)];
        if (R_LCDC & LCDC_TILE_SELECT)
            tile = VRAM_TILES_1 + idx * 0x10;
        else
            tile = VRAM_TILES_2 + ((idx + 0x80) % 0x100) * 0x10;
        tile += 2*py;

        // fetch first tile
        t1 = VRAM[tile] >> px;
        t2 = VRAM[tile+1] >> px;

        // loop & copy window
        end = (R_WX < 7 ? 0 : R_WX - 7) - 1;
        for (; X != end; X--)
            {
            SX[X] = 0xFE;
            if (px == 8)
                {
                // fetch next tile
                px = 0;
                WX = X - R_WX + 7;
                idx = VRAM[win_line + (WX >> 3)];
                if (R_LCDC & LCDC_TILE_SELECT)
                    tile = VRAM_TILES_1 + idx * 0x10;
                else
                    tile = VRAM_TILES_2 + ((idx + 0x80) % 0x100) * 0x10;
                tile += 2*py;
                t1 = VRAM[tile];
                t2 = VRAM[tile+1];
                }
            // copy window
            c = (t1 & 0x1) | ((t2 & 0x1) << 1);
            if (R_LY < LCD_HEIGHT && X < LCD_WIDTH)
                {
                gb_fb[R_LY][X] = c; //BGP[c];
                }
            t1 = t1 >> 1;
            t2 = t2 >> 1;
            px++;
            }
        WYC++; // advance window line
        }

    // draw sprites
    if (R_LCDC & LCDC_OBJ_ENABLE)
        {
        u8 s;
        for (s = NUM_SPRITES - 1; s != 0xFF; s--)
        //for (u8 s = 0; s < NUM_SPRITES && count < MAX_SPRITES_LINE; s++)
            {
            u8 OY = OAM[4*s + 0];
            u8 OX = OAM[4*s + 1];
            u8 OT = OAM[4*s + 2] & (R_LCDC & LCDC_OBJ_SIZE ? 0xFE : 0xFF);
            u8 OF = OAM[4*s + 3];

            // sprite is on this line
            if (R_LY + (R_LCDC & LCDC_OBJ_SIZE ? 0 : 8) < OY && R_LY + 16 >= OY)
                {
                u8 dir, start, end, shift;
                u8 py;

                count++;
                if (OX == 0 || OX >= 168)
                    continue;   // but not visible

                // y flip
                py = R_LY - OY + 16;
                if (OF & OBJ_FLIP_Y)
                    py = (R_LCDC & LCDC_OBJ_SIZE ? 15 : 7) - py;

                // fetch the tile
                t1 = VRAM[VRAM_TILES_1 + OT * 0x10 + 2*py];
                t2 = VRAM[VRAM_TILES_1 + OT * 0x10 + 2*py + 1];

                // handle x flip
                if (OF & OBJ_FLIP_X)
                    {
                    dir = 1;
                    start = (OX < 8 ? 0 : OX - 8);
                    end = MIN(OX, LCD_WIDTH);
                    shift = 8 - OX + start;
                    }
                else
                    {
                    dir = -1;
                    start = MIN(OX, LCD_WIDTH) - 1;
                    end = (OX < 8 ? 0 : OX - 8) - 1;
                    shift = OX - (start + 1);
                    }

                // copy tile
                t1 >>= shift;
                t2 >>= shift;
                for (X = start; X != end; X += dir)
                    {
                    c = (t1 & 0x1) | ((t2 & 0x1) << 1);
                    // check transparency / sprite overlap / background overlap
                    if (c && OX <= SX[X] &&
                        !((OF & OBJ_PRIORITY) && 
                        (R_LY < LCD_HEIGHT && X < LCD_WIDTH && 
                        (gb_fb[R_LY][X] & 0x3) && SX[X] == 0xFE)))
                        {
                        SX[X] = OX;
                        gb_fb[R_LY][X] = (OF & OBJ_PALETTE) ? OBJP[c + 4] : OBJP[c];
                        }
                    t1 = t1 >> 1;
                    t2 = t2 >> 1;
                    }
                }
            }
        }

        if (R_LY < LCD_HEIGHT)
            {
            for (X = 0; X < LCD_WIDTH; X++)
                if (SX[X] == 0xFE)
                    gb_fb[R_LY][X] = BGP[gb_fb[R_LY][X]];
            }
    }


// framebuffer
var fb;
var gb_fb = [];

for (var y = 0; y < LCD_HEIGHT; y++)
	{
	gb_fb[y] = [];
	for (var x = 0; x < LCD_WIDTH; x++)
		gb_fb[y][x] = 0;
	}

// color scheme	
COLORS = [[0xFF, 0xFF, 0xFF, 0xFF], [0x99, 0x99, 0x99, 0xFF], [0x44, 0x44, 0x44, 0xFF], [0x00, 0x00, 0x00, 0xFF]];

function DoFrame()
	{
	RunFrame();
	CopyFramebuffer();
	setTimeout(DoFrame, 0);
	}

function CopyFramebuffer()
	{
	fb = screen.getImageData(0, 0, LCD_WIDTH, LCD_HEIGHT);
	
	var c = 0;
	var p = 0;
	for (var y = 0; y < LCD_HEIGHT; y++)
		{
		for (var x = 0; x < LCD_WIDTH; x++)
			{
			c = gb_fb[y][x];
			for (var i = 0; i < 4; i++)
				fb.data[p++] = COLORS[c][i];
			}
		}
	
	screen.putImageData(fb, 0, 0);	
	}
	
// VIDEO ////////////////////////////
var WY, WYC, WX;
var X, Y;
var gb_frameskip = 0;
var gb_framecount = 0;

/*
void SetFrameSkip(u8 f)
    {
    gb_frameskip = f;
    gb_framecount = 0;
    }
*/
	
function LCDStart()
    {
    if (gb_frameskip)
        {
        gb_framecount = (gb_framecount + 1) % gb_frameskip;
        if (gb_framecount)
            return;
        }

    WY = R_WY;
    WYC = 0;

	for (var y = 0; y < LCD_HEIGHT; y++)
		for (var x = 0; x < LCD_WIDTH; x++)
			gb_fb[y][x] = 0;
    // clear tha screen
    //if (cgb_enable)
    //    memset(cgb_fb, 0x00, LCD_WIDTH * LCD_HEIGHT * 2);
    //else
    //    memset(gb_fb, 0x00, LCD_WIDTH * LCD_HEIGHT);
    }

function MIN(a, b) {if (a < b) return a; return b;}

var BX;
var BY;
var SX = []; //[LCD_WIDTH];
// to improve later: [at least it kinda works]
function LCDDrawLine()
    {
    var bg_line;
    var win_line;
    var tile;
    var t1, t2, c;
    var count = 0;

    if (R_LCDC & LCDC_BG_ENABLE)
        {
        BY = (R_LY + R_SCY) % 0x100;
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

    // memset(SX, 0xFF, LCD_WIDTH);
	for (var i = 0; i < LCD_WIDTH; i++)
		SX[i] = 0xFF;

    // draw background
    if (bg_line)
        {
        X = LCD_WIDTH - 1;
        BX = (X + R_SCX) % 0x100;

        // lookup tile index
        var py = (BY & 0x07);
        var px = 7 - (BX & 0x07);
        var idx = VRAM[bg_line + (BX >> 3)];
        if (R_LCDC & LCDC_TILE_SELECT)
            tile = VRAM_TILES_1 + idx * 0x10;
        else
            tile = VRAM_TILES_2 + ((idx + 0x80) % 0x100) * 0x10;
        tile += 2*py;

        // fetch first tile
        t1 = VRAM[tile] >> px;
        t2 = VRAM[tile+1] >> px;
        for (; X >= 0; X--)
            {
            SX[X] = 0xFE;
            if (px == 8)
                {
                // fetch next tile
                px = 0;
                BX = (X + R_SCX) % 0x100;
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
            gb_fb[R_LY][X] = c;// BGP[c];
            t1 = t1 >> 1;
            t2 = t2 >> 1;
            px++;
            }
        }

    // draw window
    if (win_line)
        {
        X = LCD_WIDTH - 1;
        WX = (X - R_WX + 7) % 0x100;

        // look up tile
        var py = WYC & 0x07;
        var px = 7 - (WX & 0x07);
        var idx = VRAM[win_line + (WX >> 3)];
        if (R_LCDC & LCDC_TILE_SELECT)
            tile = VRAM_TILES_1 + idx * 0x10;
        else
            tile = VRAM_TILES_2 + ((idx + 0x80) % 0x100) * 0x10;
        tile += 2*py;

        // fetch first tile
        t1 = VRAM[tile] >> px;
        t2 = VRAM[tile+1] >> px;

        // loop & copy window
        var end = (R_WX < 7 ? 0 : R_WX - 7);
        for (; X != end; X--)
            {
            SX[X] = 0xFE;
            if (px == 8)
                {
                // fetch next tile
                px = 0;
                WX = (X - R_WX + 7) % 0x100;
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
            gb_fb[R_LY][X] = c; //BGP[c];
            t1 = t1 >> 1;
            t2 = t2 >> 1;
            px++;
            }
        WYC++; // advance window line
        }

    // draw sprites
    if (R_LCDC & LCDC_OBJ_ENABLE)
        {
        for (var s = NUM_SPRITES - 1; s >= 0; s--)
            {
            var OY = OAM[4*s + 0];
            var OX = OAM[4*s + 1];
            var OT = OAM[4*s + 2] & (R_LCDC & LCDC_OBJ_SIZE ? 0xFE : 0xFF);
            var OF = OAM[4*s + 3];

            // sprite is on this line
            if (R_LY + (R_LCDC & LCDC_OBJ_SIZE ? 0 : 8) < OY && R_LY + 16 >= OY)
                {
                count++;
                if (OX == 0 || OX >= 168)
                    continue;   // but not visible

                // y flip
                var py = R_LY - OY + 16;
                if (OF & OBJ_FLIP_Y) 
                    py = (R_LCDC & LCDC_OBJ_SIZE ? 15 : 7) - py;

                // fetch the tile
                t1 = VRAM[VRAM_TILES_1 + OT * 0x10 + 2*py];
                t2 = VRAM[VRAM_TILES_1 + OT * 0x10 + 2*py + 1];

                // handle x flip
                var dir, start, end, shift;
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
                        !((OF & OBJ_PRIORITY) && ((gb_fb[R_LY][X] & 0x3) && SX[X] == 0xFE)))
//                    if (c && OX <= SX[X] && !(OF & OBJ_PRIORITY && gb_fb[R_LY][X] & 0x3))
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

        for (X = 0; X < LCD_WIDTH; X++)
            if (SX[X] == 0xFE)
                gb_fb[R_LY][X] = BGP[gb_fb[R_LY][X]];
    }


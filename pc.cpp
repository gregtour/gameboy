/* *****************
 * GameBoy emulator written in C.
 * Credits: Greg Tourville
********************/
// v0.02

// Windows dialog
#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>
#include "SDL_syswm.h"

// File IO
#include <cstdio>

// SDL
#include "SDL.h"
#include "SDL_main.h"

// GameBoy hardware
#include "gameboy.h"

// emulator data
bool running = true;
SDL_Event event;
u8   frameskip = 0;
u8   frames;
u32  f0_ticks;
u32  f1_ticks;
u16  fps;

// screen surfaces
SDL_Surface* screen = NULL;
const int SCR_WIDTH = 640;
const int SCR_HEIGHT = 576;
u32 fb[LCD_HEIGHT][LCD_WIDTH];

// color scheme
u32 COLORS_Y[4] = {0xFFFFFFFF, 0x99999999, 0x44444444, 0x00000000};
u32 COLORS_R[4] = {0xFFFFFFFF, 0xFFFF9999, 0xFF444499, 0x00000000};
u32 COLORS_G[4] = {0xFFFFFFFF, 0xFF99FF99, 0xFF994444, 0x00000000};
u32 COLORS_B[4] = {0xFFFFFFFF, 0xFF9999FF, 0xFF449944, 0x00000000};
u32 COLORS_O[4] = {0xFFFFFFEE, 0xFFFFFF66, 0xFF444499, 0x00000000};
u32* color_map;

// gameboy color conversion
u32 ColorTo32(u16 cgb)
    {
    u8 r = (cgb & 0x001F) << 3;// * 0xFF / 0x1F;
    u8 g = ((cgb >>  5) & 0x001F) << 3;// * 0xFF / 0x1F;
    u8 b = ((cgb >> 10) & 0x001F) << 3;// * 0xFF / 0x1F;

	//cy = (299*r + 587*g + 114*b) / 1000;
	//cb = (-16874*r - 33126*g + 50000*b + 12800000) / 100000;
	//cr = (50000*r - 41869*g - 8131*b + 12800000) / 100000;
			
			//*v0++ = *v1++ = (cy<<24) | (cb<<16) | (cy<<8) | cr;
    return 0xFF000000 | (r << 16) | (g << 8) | b;
    }

// key mappings
#define NUM_KEYS    8
u32 KEYS[] =
    {
    SDLK_RIGHT, // control map one
    SDLK_LEFT,
    SDLK_UP,
    SDLK_DOWN,
    SDLK_z,
    SDLK_x,
    SDLK_RSHIFT,
    SDLK_RETURN,
    SDLK_d,     // control map two
    SDLK_a,
    SDLK_w,
    SDLK_s,
    SDLK_SPACE,
    SDLK_BACKSPACE,
    SDLK_LSHIFT,
    SDLK_ESCAPE
    };

char  window_caption[100];
char  window_caption_fps[100];
char  rom_file_buf[260];
char* rom_file = rom_file_buf;
char  save_file[260];

u8*   rom;
u32   rom_size;
u8*   save;
u32   save_size;
FILE* rom_f;
FILE* save_f;

void PickROM()
    {
    OPENFILENAME ofn;       // common dialog box structure
    HWND hWnd = NULL;
    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    if(SDL_GetWMInfo(&wmInfo)) 
        hWnd = wmInfo.window;

    // Initialize OPENFILENAME
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWnd;
    rom_file[0] = '\0';
    ofn.lpstrFile = rom_file;
    ofn.nMaxFile = sizeof(rom_file_buf);
    ofn.lpstrFilter = "All\0*.*\0GB\0*.GB\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    // Display the Open dialog box. 
    if (!GetOpenFileName(&ofn))
        rom_file[0] = '\0';
    }

int main(int argc, char **argv)
    {
    // Init SDL
    SDL_Init(SDL_INIT_VIDEO);
    screen = SDL_SetVideoMode(SCR_WIDTH, SCR_HEIGHT, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
    SDL_WM_SetCaption("the gb", 0);

    // File chooser
    if (argc > 1)
        rom_file = argv[1];
    else
        PickROM();

    if (!rom_file || !rom_file[0])
        return 0;
    else
        {
        char* s = rom_file;
        for (u16 i = 0; rom_file[i] != '\0'; i++)
            {
            if (rom_file[i] == '\\' || rom_file[i] == '/')
                s = &rom_file[i+1];
            }
        sprintf(window_caption, "the gb - %s", s);
        SDL_WM_SetCaption(window_caption, 0);
        }

    // Load ROM file
    rom_f = fopen(rom_file, "rb");
    fseek(rom_f, 0, SEEK_END);
    rom_size = ftell(rom_f);
    rewind(rom_f);
    rom = new u8[rom_size];
    for (int i = 0; i < rom_size; i++)
        rom[i] = 0xFF;
    u32 romread = fread(rom, sizeof(u8), rom_size, rom_f);
    fclose(rom_f);

    // Load SAVE file (if it exists)
    sprintf(save_file, "%s.sav", rom_file);
    save_size = GetSaveSize(rom);
    save = new u8[save_size];
    save_f = fopen(save_file, "rb");
    if (save_f)
        {
        //save_size -= 1;
        fseek(save_f, 0, SEEK_END);
        u32 thefilesize = ftell(save_f);
        fseek(save_f,0,SEEK_SET);
        u32 read = fread(save, sizeof(u8), save_size, save_f);
        fclose(save_f);
        }

    // Fire up the emulator
    LoadROM(rom, rom_size, save, save_size);

    color_map = COLORS_Y;

    u32 old_ticks;
    u32 new_ticks = SDL_GetTicks();
    f1_ticks = new_ticks;
    int delay;
    while (running)
        {
        // handle input / events
        if (SDL_PollEvent(&event))
            {
            if (event.type == SDL_QUIT)
                running = false;
            if (event.type == SDL_KEYDOWN)
                {
                if (event.key.keysym.sym == SDLK_0)
                    {
                    frameskip = 0;
                    SetFrameSkip(0);
                    }
                else if (event.key.keysym.sym == SDLK_1)
                    {
                    frameskip = 1;
                    SetFrameSkip(0);
                    }
                else if (event.key.keysym.sym == SDLK_2)
                    {
                    frameskip = 2;
                    SetFrameSkip(2);
                    }
                else if (event.key.keysym.sym == SDLK_3)
                    {
                    frameskip = 3;
                    SetFrameSkip(3);
                    }
                else if (event.key.keysym.sym == SDLK_9)
                    {
                    frameskip = 9;
                    SetFrameSkip(10);
                    }
                else if (event.key.keysym.sym == SDLK_r)
                    color_map = COLORS_R;
                else if (event.key.keysym.sym == SDLK_g)
                    color_map = COLORS_G;
                else if (event.key.keysym.sym == SDLK_b)
                    color_map = COLORS_B;
                else if (event.key.keysym.sym == SDLK_o)
                    color_map = COLORS_O;
                else if (event.key.keysym.sym == SDLK_y)
                    color_map = COLORS_Y;
                else
                    for (u8 j = 0; j < 2*NUM_KEYS; j++)
                        if (KEYS[j] == event.key.keysym.sym)
                            {
                            KeyPress(j%NUM_KEYS);
                            break;
                            }
                }
            else if(event.type == SDL_KEYUP)
                {
                for (u8 j = 0; j < 2*NUM_KEYS; j++)
                    if (KEYS[j] == event.key.keysym.sym)
                        {
                        KeyRelease(j%NUM_KEYS);
                        break;
                        }
                }
            }

        old_ticks = SDL_GetTicks();

        // emulate frame
        RunFrame();

        if (gb_framecount == 0)
            {
            // convert colors
            if (cgb_enable)
                for (int y = 0; y < LCD_HEIGHT; y++)
                    for (int x = 0; x < LCD_WIDTH; x++)
                        fb[y][x] = ColorTo32(cgb_fb[y][x]);
            else
                for (int y = 0; y < LCD_HEIGHT; y++)
                    for (int x = 0; x < LCD_WIDTH; x++)
                        fb[y][x] = color_map[gb_fb[y][x] & 3];

            // Render
            SDL_LockSurface(screen);

            // copy framebuffer
            u32* s = (u32*)screen->pixels;
            for (int y = 0; y < SCR_HEIGHT; y++)
                {
                for (int x = 0; x < SCR_WIDTH; x++)
                    *(s + x) = fb[y/4][x/4];
                s += screen->pitch/4;
                }

            // flip screen
            SDL_UnlockSurface(screen);
            SDL_Flip(screen);

            //old_ticks = new_ticks;
            new_ticks = SDL_GetTicks();
            frames++;
            if (frames % 0x80 == 0)
                {
                f0_ticks = f1_ticks;
                f1_ticks = new_ticks;
                fps = (128*1000)/(f1_ticks - f0_ticks) * (gb_frameskip ? gb_frameskip : 1);
                sprintf(window_caption_fps, "%s - %u fps", window_caption, fps);
                SDL_WM_SetCaption(window_caption_fps, 0);
                }

            // Cap at 60 fps unless using frameskip
            if (!frameskip)
                {
                delay = 16 - (new_ticks - old_ticks);
                SDL_Delay(delay > 0 ? delay : 0);
                }
            }
        }

    // Save game before exit
    if (save_size) // Don't make blank files
        {
        save_f = fopen(save_file, "wb");
        if (save_f)
            {
            fseek(save_f,0,SEEK_SET);
            u32 written = fwrite(save, 1, save_size, save_f);
            fclose(save_f);
            }
        }

    // clean up
    delete[] rom;
    delete[] save;
    SDL_Quit();

    return 0;
    }


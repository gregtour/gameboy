/* *****************
 * GameBoy emulator written in C.
 * Credits: Greg Tourville
********************/
// v0.04

// File IO
#include <stdio.h>
#include <stdlib.h>

// SDL
#include "SDL.h"
#include "SDL_main.h"

// GameBoy
#include "gameboy.h"
#include "sound.h"

// emulator data
int running = 1;
SDL_Event event;
u8   frameskip = 0;
u8   frames;
u32  f0_ticks;
u32  f1_ticks;
u16  fps;

// screen space
SDL_Surface* screen = NULL;
#if SDL_1_2
#else
SDL_Window* window;
SDL_Renderer* renderer;
SDL_Texture* screen_tex = NULL;
#endif

const int SCR_WIDTH = 320;
const int SCR_HEIGHT = 288;
u32 fb[LCD_HEIGHT][LCD_WIDTH];

// color schemes
u32 COLORS_Y[4] = {0xFFFFFFFF, 0x99999999, 0x44444444, 0x00000000};
u32 COLORS_R[4] = {0xFFFFFFFF, 0xFFFF9999, 0xFF444499, 0x00000000};
u32 COLORS_G[4] = {0xFFFFFFFF, 0xFF99FF99, 0xFF994444, 0x00000000};
u32 COLORS_B[4] = {0xFFFFFFFF, 0xFF9999FF, 0xFF449944, 0x00000000};
u32 COLORS_O[4] = {0xFFFFFFEE, 0xFFFFFF66, 0xFF444499, 0x00000000};
u32* color_map;

// gameboy color conversion
u32 ColorTo32(u16 cgb)
    {
    u8 r = (cgb & 0x001F) << 3;
    u8 g = ((cgb >>  5) & 0x001F) << 3;
    u8 b = ((cgb >> 10) & 0x001F) << 3;

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

// strings
char  window_caption[100];
char  window_caption_fps[100];
char  rom_file_buf[260];
char* rom_file = rom_file_buf;
char  save_file[260];

// pointers
u8*   rom;
u32   rom_size;
u8*   save;
u32   save_size;
FILE* rom_f;
FILE* save_f;


int SDL_main(int argc, char **argv)
    {
    int     i, x, y;
    u8      j;
    u32     romread;
    u32     old_ticks;
    u32     new_ticks;
    int     delay;
    u32*    s;
    int     quit_seq;

    // Init SDL
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
#if SDL_1_2
    screen = SDL_SetVideoMode(SCR_WIDTH, SCR_HEIGHT, 32, SDL_HWSURFACE | SDL_DOUBLEBUF /*| SDL_FULLSCREEN*/);
    SDL_WM_SetCaption("GameBoy", 0);
#else
    window = SDL_CreateWindow("GameBoy", SDL_WINDOWPOS_CENTERED, 
                                SDL_WINDOWPOS_CENTERED,
                                SCR_WIDTH, SCR_HEIGHT,
                                SDL_WINDOW_SHOWN | SDL_WINDOW_INPUT_FOCUS);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
    screen_tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCR_WIDTH, SCR_HEIGHT);

    screen = SDL_CreateRGBSurface(0, SCR_WIDTH, SCR_HEIGHT, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
#endif


    // Start Audio
    SDLAudioStart();

    // File selection
    if (argc > 1)
        rom_file = argv[1];
    else
        return 1;

    // Need to specify ROM
    if (!rom_file || !rom_file[0])
        return 0;
    else
        {
        char* s = rom_file;
        for (i = 0; rom_file[i] != '\0'; i++)
            {
            if (rom_file[i] == '\\' || rom_file[i] == '/')
                s = &rom_file[i+1];
            }
        sprintf(window_caption, "GameBoy - %s", s);
#if SDL_1_2
        SDL_WM_SetCaption(window_caption, 0);
#else
        SDL_SetWindowTitle(window, window_caption);
#endif
        }

    // Load ROM file
    rom_f = fopen(rom_file, "rb");
    fseek(rom_f, 0, SEEK_END);
    rom_size = ftell(rom_f);
    rewind(rom_f);
    rom = (u8*)malloc(rom_size);
    for (i = 0; i < rom_size; i++)
        rom[i] = 0xFF;

    romread = fread(rom, sizeof(u8), rom_size, rom_f);
    fclose(rom_f);

    // Load SAVE file (if it exists)
    sprintf(save_file, "%s.sav", rom_file);
    save_size = GetSaveSize(rom);
    save = (u8*)malloc(save_size);
    save_f = fopen(save_file, "rb");
    if (save_f)
        {
        fseek(save_f,0,SEEK_SET);
        fread(save, sizeof(u8), save_size, save_f);
        fclose(save_f);
        }

    // Start the emulator
    LoadROM(rom, rom_size, save, save_size);

    color_map = COLORS_Y;

    new_ticks= SDL_GetTicks();
    f1_ticks = new_ticks;
    quit_seq = 0;
    while (running && quit_seq != 3)
        {
        // handle input / events
        while (SDL_PollEvent(&event))
            {
            if (event.type == SDL_QUIT)
                running = 0;
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
                else if (event.key.keysym.sym == SDLK_q)
                    quit_seq |= 1;
                else if (event.key.keysym.sym == SDLK_LCTRL)
                    quit_seq |= 2;
                else
                    for (j = 0; j < 2*NUM_KEYS; j++)
                        if (KEYS[j] == event.key.keysym.sym)
                            {
                            KeyPress(j%NUM_KEYS);
                            break;
                            }
                }
            else if(event.type == SDL_KEYUP)
                {
                for (j = 0; j < 2*NUM_KEYS; j++)
                    if (KEYS[j] == event.key.keysym.sym)
                        {
                        KeyRelease(j%NUM_KEYS);
                        break;
                        }
                if (event.key.keysym.sym == SDLK_q 
                        || event.key.keysym.sym == SDLK_LCTRL)
                    {
                    quit_seq = 0;
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
                for (y = 0; y < LCD_HEIGHT; y++)
                    for (x = 0; x < LCD_WIDTH; x++)
                        fb[y][x] = ColorTo32(cgb_fb[y][x]);
            else
                for (y = 0; y < LCD_HEIGHT; y++)
                    for (x = 0; x < LCD_WIDTH; x++)
                        fb[y][x] = color_map[gb_fb[y][x] & 3];


            // render
            SDL_LockSurface(screen);

            // copy framebuffer
            s = (u32*)screen->pixels;
            for (y = 0; y < SCR_HEIGHT; y++)
                {
                for (x = 0; x < SCR_WIDTH; x++)
                    *(s + x) = fb[y*LCD_HEIGHT/SCR_HEIGHT][x*LCD_WIDTH/SCR_WIDTH];
                s += screen->pitch/4;
                }

            // flip screen
            SDL_UnlockSurface(screen);
#if SDL_1_2
            SDL_Flip(screen);
#else
            SDL_UpdateTexture(screen_tex, NULL, screen->pixels, screen->pitch);

            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, screen_tex, NULL, NULL);
            SDL_RenderPresent(renderer);
#endif

            //old_ticks = new_ticks;
            new_ticks = SDL_GetTicks();
            frames++;
            if (frames % 0x80 == 0)
                {
                f0_ticks = f1_ticks;
                f1_ticks = new_ticks;
                fps = (128*1000)/(f1_ticks - f0_ticks) * (gb_frameskip ? gb_frameskip : 1);
                sprintf(window_caption_fps, "%s - %u fps", window_caption, fps);
#if SDL_1_2
                SDL_WM_SetCaption(window_caption_fps, 0);
#else
                SDL_SetWindowTitle(window, window_caption_fps);
#endif
                }

            // Cap at 60FPS unless using frameskip
            if (!frameskip)
                {
                delay = 16 - (new_ticks - old_ticks);
                SDL_Delay(delay > 0 ? delay : 0);
                }
            }
        }

    // Save game before exit
    if (save_size)        {
        save_f = fopen(save_file, "wb");
        if (save_f)
            {
            fseek(save_f,0,SEEK_SET);
            fwrite(save, 1, save_size, save_f);
            fclose(save_f);
            }
        }

#if SDL_2_0
    SDL_FreeSurface(screen);
#endif

    // Clean up
    free(rom);
    free(save);
    SDL_Quit();

    return 0;
    }

/* *****************
 * GameBoy emulator written in C.
 * Credits: Greg Tourville
********************/
// v0.04

// File IO
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

// SDL
#include "SDL.h"
#include "SDL_main.h"

// GameBoy
#include "gameboy.h"
#include "sound.h"

// Runtime
#include "platform.h"
#include "config.h"
#include "inspector.h"


struct {
    u8      enabled;
    u8      paused;
    u16*    breakpoints;
    u16     breakpoints_count;
    u16     breakpoints_size;
} debugger = {0, 0, NULL, 0, 0};

// emulator data
int running = 1;
SDL_Event event;
u8   frameskip = 0;
u8   frames;
u32  f0_ticks;
u32  f1_ticks;
u16  fps;

u32* color_map;

// screen space
SDL_Surface* screen_surface = NULL;

SDL_Window* window;
SDL_Renderer* renderer;
SDL_Texture* screen_tex = NULL;

u32 fb[LCD_HEIGHT][LCD_WIDTH];


// gameboy color conversion
u32 ColorTo32(u16 cgb)
{
    u8 r = (cgb & 0x001F) << 3;
    u8 g = ((cgb >>  5) & 0x001F) << 3;
    u8 b = ((cgb >> 10) & 0x001F) << 3;

    return 0xFF000000 | (r << 16) | (g << 8) | b;
}

// strings
char  window_caption[100];
char  window_caption_fps[100];
char* rom_file = NULL;
char* rom_name = NULL;
char  save_file[260];
char* bios_path = NULL;
u8    force_cgb = 0;

// pointers
u8*   rom = NULL;
u32   rom_size = 0;
u8*   save = NULL;
u32   save_size = 0;
FILE* rom_f = NULL;
FILE* save_f = NULL;

// input
SDL_GameController* controller = NULL;

/*
    Set up SDL to play sound.
*/
void SDLFillAudio(void* udata, u8* stream, int len);
void SDLAudioStart()
{
#if defined(SAVE_AUDIO_DATA_RAW) || defined(SAVE_AUDIO_DATA_SDL)
    raw = fopen("out/sound.out", "w");
#endif

    SDL_AudioSpec wanted;
    wanted.freq = SAMPLING_RATE;
    wanted.format = AUDIO_S16;
    wanted.channels = AUDIO_CHANNELS;
    wanted.samples = SAMPLING_SIZE;
    wanted.callback = SDLFillAudio;
    wanted.userdata = NULL;

    if (SDL_OpenAudio(&wanted, NULL) < 0)
    {
        printf("Error opening audio.\n");
    }
    printf("Opened audio for playback.\n");
    SDL_PauseAudio(0);
}

/*
    Provide sound to SDL audio.
*/
void SDLFillAudio(void* udata, u8* stream_u8, int len)
{
    s16* stream = (s16*)(stream_u8);
    u32 i; u8 flag = 0;

    len = (len/2) - 1;

    for (i = 0; i < len;)
    {
        if (buffer_start != buffer_end)
        {
            stream[i++] = AUDIO_BUFFER_L[buffer_start];
            stream[i++] = AUDIO_BUFFER_R[buffer_start];
            buffer_start = (buffer_start + 1) % AUDIO_BUFFER_SIZE;
        }    
        else
        {
            flag = 1;
            stream[i++] = 0;
            stream[i++] = 0;
        }
    }
    if (flag && !debugger.enabled) 
    {
        printf("audio buffer underflow\n");
    }

#ifdef SAVE_AUDIO_DATA_SDL
    fwrite(stream, sizeof(stream[0]), len, raw);
#endif
}

void check_breakpoint()
{
    if (debugger.enabled)
    {
        int i;
        for (i = 0; i < debugger.breakpoints_count; i++)
        {
            if (PC == debugger.breakpoints[i])
            {
                debugger.paused = 1;
                return;
            }
        }
    }
}

void add_breakpoint(u16 addr)
{
    if (debugger.breakpoints_count >= debugger.breakpoints_size)
    {
        if (debugger.breakpoints)
        {
            debugger.breakpoints_size = 4;
            debugger.breakpoints = malloc(sizeof(u16) * debugger.breakpoints_size);
        }
        else
        {
            debugger.breakpoints_size *= 2;
            debugger.breakpoints = realloc(debugger.breakpoints, sizeof(u16) * debugger.breakpoints_size);
        }
    }

    debugger.breakpoints[debugger.breakpoints_count++] = addr;
}

// file name basename
const char* base_name(const char* path)
{
    const char* name = path;
    char c;
    while ((c = *path))
    {
        path++;
        if (*path && (c == '/' || c == '\\'))
        {
            name = path;
        }
    }
    return name;
}

int parse_arguments(int argc, char **argv)
{
    int argn = 1;
    while (argn < argc)
    {
        if (strcmp(argv[argn], "--dmg") == 0)
        {
            cgb_enable = 0;
        }
        else if (strcmp(argv[argn], "--cgb") == 0)
        {
            force_cgb = 1;
        }
        else if (strcmp(argv[argn], "--debugger") == 0)
        {
            debugger.enabled = 1;
        }
        else if (strcmp(argv[argn], "--pause") == 0)
        {
            debugger.enabled = 1;
            debugger.paused = 1;
        }
        else if (strcmp(argv[argn], "--breakpoint") == 0)
        {
            u32 addr;

            if (++argn >= argc) 
            {
                return 1;
            }

            addr = strtol(argv[argn], NULL, 0x10);

            if (errno == EINVAL) 
            {
                return 1;
            }

            add_breakpoint(addr & 0xFFFF);
            debugger.enabled = 1;
        }
        else if (strcmp(argv[argn], "--bios") == 0)
        {
            if (++argn >= argc)
            {
                return 1;
            }

            bios_path = argv[argn];
        }
        else if (strcmp(argv[argn], "--no-bios") == 0)
        {
            gb_bios_enable = 0;
        }
        else
        {
            rom_file = argv[argn];
        }
        argn++;
    }

    return (rom_file == NULL || rom_file[0] == 0);
}

void print_usage(char* exe_name)
{
    printf("Usage: %s [--pause] [--breakpoint XXXX] <romfile>\n", base_name(exe_name));
}

int load()
{
    int     i;
    u32     romread;

    rom_f = fopen(rom_file, "rb");
    if (rom_f == NULL)
    {
        return 1;
    }
    fseek(rom_f, 0, SEEK_END);
    rom_size = ftell(rom_f);
    rewind(rom_f);
    rom = (u8*)malloc(rom_size);
    for (i = 0; i < rom_size; i++)
    {
        rom[i] = 0xFF;
    }
    romread = fread(rom, sizeof(u8), rom_size, rom_f);
    fclose(rom_f);
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

    if (bios_path)
    {
        FILE*   bios_f;
        bios_f = fopen(bios_path, "rb");
        if (bios_f)
        {
            fseek(bios_f, 0, SEEK_END);
            BIOS_SIZE = ftell(bios_f);
            BIOS = (u8*)malloc(BIOS_SIZE);
            for (i = 0; i < BIOS_SIZE; i++)
            {
                BIOS[i] = 0x00;
            }
            rewind(bios_f);
            fread(BIOS, sizeof(u8), BIOS_SIZE, bios_f);
            fclose(bios_f);
        }
    }
#ifdef DMG_BIOS_ENABLE
    else
    {
        BIOS = &DMG_BIOS[0];
        BIOS_SIZE = sizeof(DMG_BIOS);
    }
#endif
    return 0;
}


int main(int argc, char **argv)
{
    int     x, y;
    u8      j;
    u32     old_ticks;
    u32     new_ticks;
    int     delay;
    u32*    s;
    int     quit_seq;


    if (parse_arguments(argc, argv))
    {
        print_usage(argv[0]);
        exit(0);
    }

    if (load())
    {
        printf("Failed to load ROM: %s\n", rom_file);
        exit(1);
    }

    // Init SDL
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    window = SDL_CreateWindow("GameBoy", SDL_WINDOWPOS_CENTERED, 
                                SDL_WINDOWPOS_CENTERED,
                                WINDOW_WIDTH, WINDOW_HEIGHT,
                                SDL_WINDOW_SHOWN | SDL_WINDOW_INPUT_FOCUS);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
    screen_tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, LCD_WIDTH, LCD_HEIGHT);
    screen_surface = SDL_CreateRGBSurface(0, LCD_WIDTH, LCD_HEIGHT, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
    sprintf(window_caption, "GameBoy - %s", base_name(rom_file));
    SDL_SetWindowTitle(window, window_caption);
    printf("Num gamepads: %i\n", SDL_NumJoysticks());
    controller = SDL_GameControllerOpen(0);
    if (controller) 
    {
        printf("Gamepad connected.\n");
    }

    // Start inspector
    InspectorStartup(renderer);

    // Start Audio
    SDLAudioStart();

    // Start the emulator
    LoadROM(rom, rom_size, save, save_size);
    if (force_cgb)
    {
        cgb_enable = 1;
    }

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
            {
                running = 0;
            }
            
            if (event.type == SDL_MOUSEBUTTONUP)
            {
                if (debugger.enabled)
                {
                    InspectorClick(event.button.x, event.button.y);
                }
            }

            if (event.type == SDL_KEYDOWN)
            {
                if (event.key.keysym.sym == SDLK_F5)
                {
                    debugger.enabled = !debugger.enabled;
                }
                else if (event.key.keysym.sym == SDLK_p && debugger.enabled)
                {
                    debugger.paused = !debugger.paused;
                    if (!debugger.paused)
                    {
                        StepCPU();
                    }
                }
                else if (event.key.keysym.sym == SDLK_g && debugger.enabled)
                {
                    for (int k = 0; k < 32; k++) 
                    {
                        //BCPD[k] = 0x11 << (k % 16) | 0x8000 >> (k/2);
                        //OCPD[k] = ~(0x11 << (k % 16) | 0x8000 >> (k/2));
                        u16 tint = (0x11 << (k % 16)) | (0x80 >> (2 * (k / 4)));
                        BCPD[k] = 0xF800 ^ tint;
                        OCPD[k] = 0x001F ^ tint;
                    }

                }
                else if (event.key.keysym.sym == SDLK_n && debugger.paused)
                {
                    StepCPU();
                }
                else if (event.key.keysym.sym == SDLK_f && debugger.paused)
                {
                    RunFrame();
                }
                else if (event.key.keysym.sym == SDLK_0)
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
            else if(event.type == SDL_CONTROLLERBUTTONDOWN)
            {
                for (j = 0; j < NUM_KEYS; j++)
                {
                    if (BUTTONS[j] == event.cbutton.button)
                    {
                        KeyPress(j);
                    }
                }
            }
            else if (event.type == SDL_CONTROLLERBUTTONUP)
            {
                for (j = 0; j < NUM_KEYS; j++)
                {
                    if (BUTTONS[j] == event.cbutton.button)
                    {
                        KeyRelease(j);
                    }
                }
            }
        }

        old_ticks = SDL_GetTicks();

        // check breakpoints

        // emulate frame
        check_breakpoint();
        if (debugger.enabled)
        {
            gb_frame = 0;
            check_breakpoint();
            while (!debugger.paused && !gb_frame)
            {
                StepCPU();
                check_breakpoint();
            }
        }
        else 
        {
            RunFrame();
        }


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
            SDL_LockSurface(screen_surface);

            // copy framebuffer
            s = (u32*)screen_surface->pixels;
            for (y = 0; y < LCD_HEIGHT; y++)
            {
                for (x = 0; x < LCD_WIDTH; x++)
                    *(s + x) = fb[y][x];
                s += screen_surface->pitch/4;
            }

            // flip screen
            SDL_UnlockSurface(screen_surface);

            SDL_UpdateTexture(screen_tex, NULL, screen_surface->pixels, screen_surface->pitch);

            SDL_Rect screen_tex_rect;
            screen_tex_rect.x = 0;
            screen_tex_rect.y = 0;
            screen_tex_rect.w = LCD_WIDTH;
            screen_tex_rect.h = LCD_HEIGHT;

            SDL_Rect renderer_rect;
            if (debugger.enabled)
            {
                renderer_rect.x = 20;
                renderer_rect.y = LCD_HEIGHT;
                renderer_rect.w = LCD_WIDTH * 2;// * SCALE_FACTOR;
                renderer_rect.h = LCD_HEIGHT * 2;// * SCALE_FACTOR;
            }
            else
            {
                renderer_rect.x = 0;
                renderer_rect.y = 0;
                renderer_rect.w = WINDOW_WIDTH;// * SCALE_FACTOR;
                renderer_rect.h = WINDOW_HEIGHT;// * SCALE_FACTOR;
            }

            SDL_RenderClear(renderer);

            if (debugger.enabled)
            {   
                InspectorDraw(renderer);
            }

            SDL_RenderCopy(renderer, screen_tex, &screen_tex_rect, &renderer_rect);

            SDL_RenderPresent(renderer);

            //old_ticks = new_ticks;
            new_ticks = SDL_GetTicks();
            frames++;
            if (frames % 0x80 == 0)
            {
                f0_ticks = f1_ticks;
                f1_ticks = new_ticks;
                fps = (128*1000)/(f1_ticks - f0_ticks) * (gb_frameskip ? gb_frameskip : 1);
                sprintf(window_caption_fps, "%s - %u fps", window_caption, fps);

                SDL_SetWindowTitle(window, window_caption_fps);
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
    if (save_size)
    {
        save_f = fopen(save_file, "wb");
        if (save_f)
        {
            fseek(save_f,0,SEEK_SET);
            fwrite(save, 1, save_size, save_f);
            fclose(save_f);
        }
    }

    SDL_FreeSurface(screen_surface);

    // Clean up
    free(debugger.breakpoints);
    free(rom);
    free(save);
    SDL_Quit();

    return 0;
}

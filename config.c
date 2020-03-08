
#include "SDL.h"
#include "config.h"

// color schemes
u32 COLORS_Y[4] = {0xFFFFFFFF, 0x99999999, 0x44444444, 0x00000000};
u32 COLORS_R[4] = {0xFFFFFFFF, 0xFFFF9999, 0xFF444499, 0x00000000};
u32 COLORS_G[4] = {0xFFFFFFFF, 0xFF99FF99, 0xFF994444, 0x00000000};
u32 COLORS_B[4] = {0xFFFFFFFF, 0xFF9999FF, 0xFF449944, 0x00000000};
u32 COLORS_O[4] = {0xFFFFFFEE, 0xFFFFFF66, 0xFF444499, 0x00000000};

// key mappings
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

u32 BUTTONS[] =
{
    SDL_CONTROLLER_BUTTON_DPAD_RIGHT,
    SDL_CONTROLLER_BUTTON_DPAD_LEFT,
    SDL_CONTROLLER_BUTTON_DPAD_UP,
    SDL_CONTROLLER_BUTTON_DPAD_DOWN,
    SDL_CONTROLLER_BUTTON_A,
    SDL_CONTROLLER_BUTTON_B,
    SDL_CONTROLLER_BUTTON_BACK,
    SDL_CONTROLLER_BUTTON_START ,
};

// gameboy color conversion
u32 ColorTo32(u16 cgb)
{
    u8 r = (cgb & 0x001F) << 3;
    u8 g = ((cgb >>  5) & 0x001F) << 3;
    u8 b = ((cgb >> 10) & 0x001F) << 3;

    return 0xFF000000 | (r << 16) | (g << 8) | b;
}

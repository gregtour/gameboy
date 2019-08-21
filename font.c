
#include "gameboy.h"
#include "font.h"

const char* bitmap_font_chars = 
    " !\"#$%&'()"   "*+,-./0123"
    "456789:;<="    ">?@ABCDEFG"
    "HIJKLMNOPQ"    "RSTUVWXYZ["
    "\\]^_`abcde"   "fghijklmno"
    "pqrstuvwxy"    "z{|}~";

const uint64_t bitmap_font_pixels[64] = 
{
    0x1081044de28a100,  0x20408a2c57ca100,  0x20400410e280100,  0x20400a6947c0000,
    0x10801464f280100,  0x0,                0x18c10e400000015,  0x21219920000010e,
    0x10811510038039f,  0x20411308000410e,  0x19e38e044002015,  0x0,
    0x800018678438c,    0x38410424940208a,  0x200038620719e,    0x384104209109208,
    0x8080206106188,    0x0,                0x79f7cf78f38e182,  0x41051051450204,
    0x64f3d104f7d6108,  0x441051051455004,  0x7817cf78f44e102,  0x0,
    0x38f39144125c391,  0x4514536c1148111,  0x44f4555410c811f,  0x241459441149111,
    0x58139144f246391,  0x0,                0x38e45145145f78f,  0x8828a451444051,
    0x8410455144438f,   0x8210a54a444405,   0x38e1112843843c9,  0x0,
    0x8001004004381,    0x10830738800a202,  0x28e089240000204,  0x189089240000208,
    0x30e3075807c0390,  0x0,                0x4088102308,       0x1062c4080002284,
    0x28a54428810e30e,  0x28a44418a112204,  0x10a444284112184,  0x0,
    0x0,                0x28a44a48e30a306,  0x28454a48410628a,  0x10454a484202306,
    0x8a284308302202,   0x0,                0x1fc07c0084200,    0x1fc044a10410c,
    0x1fc0445300188,    0x1fc0440104104,    0x1fff7c008420c,    0x1fff000000000,
    0x0,                0x0,                0x0,                0x0
};

SDL_Texture* font_texture = NULL;
SDL_Surface* font_surface = NULL;

void FontStartup(SDL_Renderer* renderer)
{
    u32 x, y;
    u32* itr;

    font_texture = SDL_CreateTexture(
        renderer, 
        SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 
        FONT_BITMAP_WIDTH, FONT_BITMAP_HEIGHT);

    font_surface = SDL_CreateRGBSurface(0, FONT_BITMAP_WIDTH, FONT_BITMAP_HEIGHT, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);

    SDL_LockSurface(font_surface);

    itr = (u32*)font_surface->pixels;
    for (y = 0; y < FONT_BITMAP_WIDTH; y++)
    {
        for (x = 0; x < FONT_BITMAP_HEIGHT; x++)
        {
            u32 color, bit;
            bit = (bitmap_font_pixels[y] >> x) & 0x01;
            color = (bit ? TEXT_FOREGROUND : TEXT_BACKGROUND);
            *(itr + x) = color;
        }
        itr += font_surface->pitch/4;
    }

    SDL_UnlockSurface(font_surface);

    SDL_UpdateTexture(font_texture, NULL, font_surface->pixels, font_surface->pitch);

    SDL_SetTextureBlendMode(font_texture, SDL_BLENDMODE_BLEND);
}


void FontPrint(SDL_Renderer* renderer, const char* text, int x, int y)
{
    SDL_Rect font_rect, screen_rect;
    
    font_rect.w = FONT_CHAR_WIDTH; 
    font_rect.h = FONT_CHAR_HEIGHT;

    screen_rect.w = FONT_CHAR_WIDTH * FONT_RENDER_SCALE;
    screen_rect.h = FONT_CHAR_HEIGHT * FONT_RENDER_SCALE;
    screen_rect.x = x;
    screen_rect.y = y;

    for (; *text; text++)
    {
        u32 pos;
        const char* seek = bitmap_font_chars;
        char lookup = *text;
        for (; *seek; seek++)
        {
            if (*seek == lookup) break;
        }

        if (!seek) continue;

        pos = seek - bitmap_font_chars;

        font_rect.x = (pos % FONT_COLUMNS) * FONT_CHAR_WIDTH;
        font_rect.y = (pos / FONT_COLUMNS) * FONT_CHAR_HEIGHT;

        SDL_RenderCopy(renderer, font_texture, &font_rect, &screen_rect);

        screen_rect.x += screen_rect.w;
    }
}




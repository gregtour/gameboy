/* Audio Support */
#include "gameboy.h"
#include "sound.h"

#include <stdio.h>
#include <math.h>

// sound registers
extern u8 R_NR10;
extern u8 R_NR11;
extern u8 R_NR12;
extern u8 R_NR13;
extern u8 R_NR14;
extern u8 R_NR21;
extern u8 R_NR22;
extern u8 R_NR24;
extern u8 R_NR30;
extern u8 R_NR31;
extern u8 R_NR32;
extern u8 R_NR33;
extern u8 R_NR41;
extern u8 R_NR42;
extern u8 R_NR43;
extern u8 R_NR44;
extern u8 R_NR50;
extern u8 R_NR51;
extern u8 R_NR52;

void fill_audio(void* udata, u8* stream, int len);

void SDLAudioStart()
    {
    SDL_AudioSpec wanted;
    wanted.freq = 22050;
    wanted.format = AUDIO_S16;
    wanted.channels = 2;
    wanted.samples = 1024;
    wanted.callback = fill_audio;
    wanted.userdata = NULL;

    if (SDL_OpenAudio(&wanted, NULL) < 0)
        printf("Error opening audio.\n");
    printf("Opened audio for playback.\n");
    SDL_PauseAudio(0);
    }

void fill_audio(void* udata, u8* stream, int len)
    {
    int i; char hi; char lo;
    u8 a; u8 b; u8 c; u8 d;
    u8 SO1; u8 SO2;

    for (i = 0; i < len; i++)
        {
        int f1 = (R_NR14 & 7) << 8 | R_NR13;
        stream[i] = sinf(i * 4194304 / 4 / (2048 - f1)) * 256 + 128;
        
        //i / (2048 - f1 + 1);
/*a = (i) * (255 - (R_NR10 ^ R_NR11 ^ R_NR12 ^ R_NR14));
        b = (i/2) * (255 - (R_NR21 ^ R_NR22 ^ R_NR24));
        c = (i/8) * (255 - (R_NR30 ^ R_NR31 ^ R_NR32 ^ R_NR33));
        d = rand() * (R_NR41 ^ R_NR42 ^ R_NR43 ^ R_NR44);

        a = a /4;
        b = b /2;
        c = c /4;
        d = d /8;
        
        SO2 = (R_NR51 & 0x80 ? d : 0) +
                (R_NR51 & 0x40 ? c : 0) +
                (R_NR51 & 0x20 ? b : 0) +
                (R_NR51 & 0x10 ? a : 0);

        SO1 = (R_NR51 & 0x08 ? d : 0) +
                (R_NR51 & 0x04 ? c : 0) +
                (R_NR51 & 0x02 ? b : 0) +
                (R_NR51 & 0x01 ? a : 0);

        stream[i] = SO2;
        
        //((R_NR50 & 0x80) ?
        //  (((R_NR50 & 0x70) >> 4) * SO2) : 0) +
        //        ((R_NR50 & 0x08) ?
        //   ((R_NR50 & 0x07) * SO1) : 0);

#if 0
        stream[i] = 0;
        hi = R_NR10 * i;
        hi += i/(R_NR11+1);
        hi = hi * R_NR12;
        hi += R_NR14 * i;
        hi = hi % (R_NR21 ? R_NR21 : 1);

        lo = R_NR22 ^ R_NR24;

        stream[i] += hi / 4;
        stream[i] += lo / 2;
        stream[i] += R_NR33;
        stream[i] += R_NR43 ? (i / (R_NR43+1)) : 0;

        stream[i] = R_NR30 ? stream[i] : 0;
#endif
*/
        }
    }


/* Audio Support */
#include "gameboy.h"
#include "sound.h"

#include <stdio.h>
#include <math.h>

// SOUND ////////////////////////////
// 
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
/*  SDL_AudioSpec wanted;
    wanted.freq = 22050;
    wanted.format = AUDIO_S16;
    wanted.channels = 2;
    wanted.samples = 1024;
    wanted.callback = fill_audio;
    wanted.userdata = NULL;

    if (SDL_OpenAudio(&wanted, NULL) < 0)
        printf("Error opening audio.\n");
    printf("Opened audio for playback.\n");
    SDL_PauseAudio(0);*/
    }

void fill_audio(void* udata, u8* stream, int len)
    {
    int i; char hi; char lo;
    u8 a; u8 b; u8 c; u8 d;
    u8 SO1; u8 SO2;

    for (i = 0; i < len; i++)
        {
        // No sound.
        stream[i] = 0;
        }
    }

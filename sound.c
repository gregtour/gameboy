/* Audio Support */
#include "gameboy.h"
#include "sound.h"

#include <stdio.h>
#include <math.h>

FILE* raw;

u8 ARAM[16];

void fill_audio(void* udata, s16* stream, int len);

void SDLAudioStart()
    {
    raw = fopen("out/sound.out", "w");
    SDL_AudioSpec wanted;
    wanted.freq = SAMPLING_RATE;
    wanted.format = AUDIO_S16;
    wanted.channels = 1;
    wanted.samples = SAMPLING_SIZE;
    wanted.callback = fill_audio;
    wanted.userdata = NULL;

    if (SDL_OpenAudio(&wanted, NULL) < 0)
        {
        printf("Error opening audio.\n");
        }
    printf("Opened audio for playback.\n");
    SDL_PauseAudio(0);
    }

u8 AUDIO_READ(u8 addr)
    {
    // sound reads don't work at all if off
    if (!(R_NR52 & NR52_ALL_SOUND) && addr != 0x26) 
        {
        return 0;
        }
    // read sound registers / data
    switch (addr)
        {
        // audio registers
        case 0x10: return R_NR10;
        case 0x11: return R_NR11 & NR11_WAVE_DUTY;
        case 0x12: return R_NR12;
        case 0x13: return /*R_NR13*/ 0;
        case 0x14: return R_NR14 & NR14_COUNTER;
        case 0x15: return /*N/A*/ R_NR20;
        case 0x16: return R_NR21 & NR21_WAVE_DUTY;
        case 0x17: return R_NR22;
        case 0x18: return /*R_NR23*/ 0;
        case 0x19: return R_NR24 & NR24_COUNTER;
        case 0x1A: return R_NR30;
        case 0x1B: return R_NR31;
        case 0x1C: return R_NR32;
        case 0x1D: return /*R_NR33*/ 0;
        case 0x1E: return R_NR34 & NR34_COUNTER;
        case 0x1F: return R_NR40;
        case 0x20: return R_NR41;
        case 0x21: return R_NR42;
        case 0x22: return R_NR43;
        case 0x23: return R_NR44 & NR44_COUNTER;
        case 0x24: return R_NR50;
        case 0x25: return R_NR51;
        case 0x26: return R_NR52;
        // wave pattern RAM
        case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37: 
        case 0x38: case 0x39: case 0x3A: case 0x3B: case 0x3C: case 0x3D: case 0x3E: case 0x3F: 
            return ARAM[(addr - 0x30)];
        }
    return 0;
    }

void AUDIO_WRITE(u8 addr, u8 val)
    {
    // sound writes don't work if all off
    if (!(R_NR52 & NR52_ALL_SOUND)) return;
    u8 n;

    // write sound register / data
    switch (addr)
        {
        case 0x10: R_NR10 = val;    
            return;
        case 0x11: 
            if (R_NR14 & NR14_COUNTER)
                {
                R_NR11 = val;
                }
            else
                {
                R_NR11 = (val & NR11_WAVE_DUTY) | (R_NR11 & NR11_SOUND_LEN);
                }
            return;
        case 0x12: R_NR12 = val;    
            return;
        case 0x13: R_NR13 = val;    
            return;
        case 0x14: R_NR14 = val;    
            if (val & NR14_INIT)
                {
                R_NR52 |= NR52_CH1_ON;
                }
            return;
        case 0x15: R_NR20 = val;    
            return;
        case 0x16: //R_NR21 = val;    
            if (R_NR24 & NR24_COUNTER)
                {
                R_NR21 = val;
                }
            else
                {
                R_NR21 = (val & NR21_WAVE_DUTY) | (R_NR21 & NR21_SOUND_LEN);
                }
            return;
        case 0x17: R_NR22 = val;    
            return;
        case 0x18: R_NR23 = val;    
            return;
        case 0x19: R_NR24 = val;    
            if (val & NR24_INIT)
                {
                R_NR52 |= NR52_CH2_ON;
                }
            return;
        case 0x1A: R_NR30 = val;    
            return;
        case 0x1B: //R_NR31 = val;    
            if (R_NR34 & NR34_COUNTER)
                {
                R_NR31 = val;
                }
            return;
        case 0x1C: R_NR32 = val;    
            return;
        case 0x1D: R_NR33 = val;    
            return;
        case 0x1E: R_NR34 = val;    
            if (val & NR34_INIT)
                {
                R_NR52 |= NR52_CH3_ON;
                }
            return;
        case 0x1F: R_NR40 = val;    
            return;
        case 0x20: //R_NR41 = val;    
            if (R_NR44 & NR44_COUNTER)
                {
                R_NR41 = val & NR41_SOUND_LEN;
                }
            return;
        case 0x21: R_NR42 = val;    
            return;
        case 0x22: R_NR43 = val;    
            return;
        case 0x23: R_NR44 = val;    
            if (val & NR44_INIT)
                {
                R_NR52 |= NR52_CH4_ON;
                }
            return;
        case 0x24: R_NR50 = val;    
            return;
        case 0x25: R_NR51 = val;    
            return;
        case 0x26: 
            R_NR52 = (val & NR52_ALL_SOUND) | (R_NR52 & NR52_STATUS);
            if (!(val & NR52_ALL_SOUND))
                {
                R_NR52 = 0;
                }
            return;
        // wave pattern RAM
        case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37: 
        case 0x38: case 0x39: case 0x3A: case 0x3B: case 0x3C: case 0x3D: case 0x3E: case 0x3F: 
            ARAM[(addr - 0x30)] = val;
            return;
        }
    }

void SoundStart() {}
void AudioUpdate() {}

u32 PERIOD(u16 freq)
    {
    //period = (SAMPLING_RATE / 1 /*/ 64*/ / 2) / freq;
    //period = period > 1 ? period : 2;
    //period = 2 * (2048 - freq) / 11;
    return (2048 - freq) / 8;
    }

s16 RESAMPLE(u32 tick, u32 period, u8 vol)
    {
    if (!period) period = 1;
    u32 N = 11 * tick;
    u32 D = 4 * period;
    u32 D2 = D / 2;
    u32 q = (N / D);
    u32 r = (N - q * D);
    if (r > D2) 
        {
        r = D - r;
        }

    s32 sample = vol * 128 * (r | (r>>1)) / D2 - vol * 64;
    s16 svol = vol << 6;
    sample = ((tick/period)%2) ? svol : -svol;

    return sample;
    }

u32 rand_state = 0x0101;
s16 NOISE_SAMPLE(u32 tick, u8 vol)
    {
    rand_state = ~(rand_state >> 5) ^ (rand_state * 113 + 17);
    return (vol << 5) * (rand_state % 32 - 16) / 16;
    }

u16 notes_on[2] = { 0 };

u32 sound_counter = 0;
extern u32 cpu_count;
void fill_audio(void* udata, s16* stream, int len)
    {
    len /= 2;

    u32 i; u32 tick; u32 period; s16 sample;
    u32 freq; u8 vol; u8 enable; u8 sndlen;

    u16 new_notes[2] = { 0 };

    rand_state ^= 1719 * cpu_count;

    for (i = 0; i < len; i++) 
        {
        stream[i] = 0;
        }

    // // audio is off
    if (!(R_NR52 & NR52_ALL_SOUND)) return;

#if 1
    // channel 1
    freq = (R_NR13 & NR13_FREQ_LO) | ((R_NR14 & NR14_FREQ_HI) << 8);
    vol = (R_NR12 & NR12_INIT_VOLUME) / 3;
    enable = (R_NR52 & NR52_CH1_ON);

    new_notes[0] = freq;

    // decrement length
    if ((R_NR14 & NR14_COUNTER) && (R_NR11 & NR11_SOUND_LEN))
        {
        sndlen = (R_NR11 & NR11_SOUND_LEN) + 1;
        R_NR11 = (R_NR11 & NR11_WAVE_DUTY) | (sndlen & NR11_SOUND_LEN);
        if (sndlen & 0x40)
            {
            R_NR52 = R_NR52 & (~NR52_CH1_ON);
            }
        }

    // play sound
    if (vol > 0 && freq > 0 && enable)
        {
        period = PERIOD(freq);
        for (i = 0; i < len; tick = sound_counter + i++)
            {
            sample = RESAMPLE(tick, period, vol);
            stream[i] += sample;
            }
        }
#endif
#if 1
    // channel 2
    freq = (R_NR23 & NR23_FREQ_LO) | ((R_NR24 & NR24_FREQ_HI) << 8);
    vol = (R_NR22 & NR22_INIT_VOLUME) / 3;
    enable = (R_NR52 & NR52_CH2_ON);

    new_notes[1] = freq;

    // decrement length
    if ((R_NR24 & NR24_COUNTER) && (R_NR21 & NR21_SOUND_LEN))
        {
        sndlen = (R_NR21 & NR21_SOUND_LEN) + 1;
        R_NR21 = (R_NR21 & NR21_WAVE_DUTY) | (sndlen & NR21_SOUND_LEN);
        if (sndlen & 0x40)
            {
            R_NR52 = R_NR52 & (~NR52_CH2_ON);
            }
        }

    if (vol > 0 && freq > 0 && R_NR52 & enable) 
        {
        period = PERIOD(freq);
        for (i = 0; i < len; tick = sound_counter + i++)
            {
            sample = RESAMPLE(tick, period, vol);
            stream[i] += sample;
            }
        }
#endif
#if 1

    vol = (R_NR42 & NR42_INIT_VOLUME) / 3;
    enable = (R_NR52 & NR52_CH2_ON);

    if ((R_NR44 & NR44_COUNTER) && (R_NR41 & NR41_SOUND_LEN))
        {
        sndlen = (R_NR41 & NR41_SOUND_LEN) + 1;
        R_NR41 = sndlen & NR41_SOUND_LEN;
        if (sndlen & 0x40)
            {
            R_NR52 = R_NR52 & (~NR52_CH4_ON);
            }
        }

    if (vol > 0 && R_NR52 & NR52_CH4_ON)
        {
        for (i = 0; i < len; tick = sound_counter + i++)
            {
            sample = NOISE_SAMPLE(tick, vol);
            stream[i] += sample;
            }
        }
#endif

    /*
    if (notes_on[0] != new_notes[0])
        {
        if (notes_on[0]) printf("off %i\n", notes_on[0]);
        if (new_notes[0]) printf("on %i\n", new_notes[0]);
        notes_on[0] = new_notes[0];
        }
    if (notes_on[1] != new_notes[1])
        {
        if (notes_on[1]) printf("off %i\n", notes_on[1]);
        if (new_notes[1]) printf("on %i\n", new_notes[1]);
        notes_on[1] = new_notes[1];
        }
    //*/
    fwrite(stream, sizeof(stream[0]), len, raw);

    sound_counter += len;
    }

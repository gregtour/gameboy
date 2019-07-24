/* Audio Support */
#include "gameboy.h"
#include "sound.h"

#include <stdio.h>
#include <math.h>

u16 buffer_start = 0;
u16 buffer_end = 0;

s16 AUDIO_BUFFER_L[AUDIO_BUFFER_SIZE];
s16 AUDIO_BUFFER_R[AUDIO_BUFFER_SIZE];

u8 ARAM[16];

void fill_audio(void* udata, u8* stream, int len);

void SDLAudioStart()
    {
    SDL_AudioSpec wanted;
    wanted.freq = SAMPLING_RATE;
    wanted.format = AUDIO_S16SYS;
    wanted.channels = 2;
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
            if (!val & NR52_ALL_SOUND) 
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

void SoundStart()
    {
    }

void SDLFillAudio(void* udata, s16* stream, int len)
    {
    len -= 1;
    for (i = 0; i < len; i += 2)
        {
        if (buffer_start != buffer_end)
            {
            stream[i+0] = AUDIO_BUFFER_L[buffer_start];
            stream[i+1] = AUDIO_BUFFER_R[buffer_start];
            buffer_start = (buffer_start + 1) % AUDIO_BUFFER_SIZE;
            }    
        else
            {
            stream[i+0] = 0;
            stream[i+1] = 0;
            }
        }
    }

u32 audio_frame = 0;
u32 audio_count = 0;
void AudioUpdate()
    {
    u32 i; u32 tick; u32 period; u8 sample;
    u32 freq; u8 vol; u8 enable; u8 sndlen;
    
    u32 fill_amt = (audio_frame % 2) ? 188 : 187;

    u32 fill_start = buffer_end;
    u32 fill_end = (buffer_end + fill_amt) % AUDIO_BUFFER_SIZE;

    if (buffer_end - buffer_start)

    // // audio is off
    if (!(R_NR52 & NR52_ALL_SOUND)) return;

#if 1
    // channel 1
    freq = (R_NR13 & NR13_FREQ_LO) | ((R_NR14 & NR14_FREQ_HI) << 8);
    vol = (R_NR12 & NR12_INIT_VOLUME) / 3;
    enable = (R_NR52 & NR52_CH1_ON);

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
        //period = (SAMPLING_RATE / 1 /*/ 64*/ / 2) / freq;
        //period = period > 1 ? period : 2;
        period = (2048 - freq) >> 2;
        if (!period) period = 1;
        for (i = 0; i < len; tick = sound_counter + i++)
            {
            sample = (tick / period) % 2 ? vol : 0;
            //sample = (sample % period) * vol / sample;
            stream[i] += sample;
            }
        }
#endif
#if 1
    // channel 2
    freq = (R_NR23 & NR23_FREQ_LO) | ((R_NR24 & NR24_FREQ_HI) << 8);
    vol = (R_NR22 & NR22_INIT_VOLUME) / 3;
    enable = (R_NR52 & NR52_CH2_ON);

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
        //period = (SAMPLING_RATE / 1 /*/ 64*/ / 2) / freq;
        //period = period > 1 ? period : 2;
        period = (2048 - freq) >> 2;
        if (!period) period = 1;
        for (i = 0; i < len; tick = sound_counter + i++)
            {
            sample = (tick / period) % 2 ? vol : 0;
            stream[i] += sample;
            }
        }
#endif

#if 0
    int i; char hi; char lo;
    u8 a; u8 b; u8 c; u8 d;
    u8 SO1; u8 SO2;

    u32 freq = R_NR13 | (R_NR14 & 0x07) << 3;
    u8  vol = 100;

    for (i = 0; i < len; i++)
        {
        u32 tick = sound_counter + i;
        if (freq)
            {
            stream[i] = (tick/freq)%2 ? vol : 0;
            }
        else
            {
            stream[i] = 0;
            }
        }
#endif

    audio_frame++;
    audio_count += fill_amt;
    }

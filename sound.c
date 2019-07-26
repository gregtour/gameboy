/* Audio Support */
#include "gameboy.h"
#include "sound.h"

#include <stdio.h>
#include <math.h>

// #define SAVE_AUDIO_DATA_RAW

s16 AUDIO_BUFFER_L[AUDIO_BUFFER_SIZE];
s16 AUDIO_BUFFER_R[AUDIO_BUFFER_SIZE];

FILE* raw = NULL;

u8 ARAM[16];

/*
    Handle reads from sound controller.
*/
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

/*
    Handle writes to sound controller.
*/
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


/*
    Set up SDL to play sound.
*/
void SDLFillAudio(void* udata, s16* stream, int len);
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
    Generate 'noise'
*/
s16 NOISE(u32 tick, u8 vol)
    {
    s16 svol = vol >> 2;
    static u32 rand_state = 0xF390439F;
    rand_state = 137 * rand_state + ((rand_state >> 8) | (rand_state << 24)) + 1;

    return svol * (rand_state & 0xFF - 128);
    }

/*
    Process GB frequency value.
*/
u32 PERIOD(u16 xfreq)
    {
    return (2048 - xfreq);
    }

/*
    Signal generator
*/
s16 RESAMPLE(u32 tick, u32 period, u8 vol)
    {
    if (!period) period = 1;

    s32 sample;
    s16 svol = vol << 6;

    u32 N = 11 * tick;
    u32 D = 4 * period;
    u32 D2 = D / 2;
    u32 q = (N / D);
    u32 r = (N - q * D);
    if (r > D2) 
        {
        r = D - r;
        sample = svol;
        }
    else
        {
        sample = -svol;
        }

    sample = vol * 128 * r / D2 - vol * 64;

    return sample;
    }

u32 audio_frame = 0;
u32 sample_count = 0;
u32 buffer_start = 0;
u32 buffer_end = 0;

/*
    Handle audio logic and generate sound at 256hz
*/
void AudioUpdate()
    {
    u32 audio_cycle = audio_frame;
    u32 i; u16 freq; u32 period; u8 vol; u8 enable; u8 sndlen; s16 sample;
    u32 fill_amt, fill_start, fill_end, fill_idx;

    u32 filled = (buffer_end - buffer_start) % AUDIO_BUFFER_SIZE;
    u32 capacity = (buffer_start - buffer_end - 1) % AUDIO_BUFFER_SIZE - SAMPLING_SIZE;

    // calculate amount of sound buffer to fill
    fill_amt = AUDIO_FILL;
    if (filled < SAMPLING_SIZE)
        {
        fill_amt += 10;
        }
    else if (capacity < 2*SAMPLING_SIZE)
        {
        fill_amt -= 10;
        }

    if (fill_amt > capacity) fill_amt = capacity;

    // warn for overflow
    if (capacity == 0) printf("audio buffer overflow\n");

    // set pointers
    fill_start = buffer_end;
    fill_end = (buffer_end + fill_amt) % AUDIO_BUFFER_SIZE;

    // clear sound buffers
    for (i = fill_start; 
        i != fill_end; 
        i = (i + 1) % AUDIO_BUFFER_SIZE)
        {
        AUDIO_BUFFER_R[i] = 0;
        AUDIO_BUFFER_L[i] = 0;
        }

    //printf("audio count: %i fill: %i\n", sample_count, fill_amt);

    // audio is off
    if (R_NR52 & NR52_ALL_SOUND)
        {

#if 1
        // channel 1
        freq = (R_NR13 & NR13_FREQ_LO) | ((R_NR14 & NR14_FREQ_HI) << 8);
        //freq = 1000;
        vol = (R_NR12 & NR12_INIT_VOLUME);

        // sound length counter
        if ((R_NR14 & NR14_COUNTER) && (R_NR11 & NR11_SOUND_LEN))
            {
            sndlen = (R_NR11 & NR11_SOUND_LEN) + 1;
            R_NR11 = (R_NR11 & NR11_WAVE_DUTY) | (sndlen & NR11_SOUND_LEN);
            if (sndlen & 0x40)
                {
                R_NR52 = R_NR52 & (~NR52_CH1_ON);
                }
            }
        // sweep
        // volume envelope
        // play sound
        enable = (R_NR52 & NR52_CH1_ON);
        if (vol > 0 && freq > 0 && enable)
            {
            period = PERIOD(freq);
            for (i = 0, fill_idx=fill_start; 
                fill_idx != fill_end; 
                i++, fill_idx=(fill_idx+1)%AUDIO_BUFFER_SIZE)
                {
                sample = RESAMPLE(sample_count+i, period, vol);
                AUDIO_BUFFER_L[fill_idx] += sample;
                AUDIO_BUFFER_R[fill_idx] += sample;
                }
            }
#endif  

#if 1
        // channel 2
        freq = (R_NR23 & NR23_FREQ_LO) | ((R_NR24 & NR24_FREQ_HI) << 8);
        vol = (R_NR22 & NR22_INIT_VOLUME) / 3;

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

        enable = (R_NR52 & NR52_CH2_ON);
        if (vol > 0 && freq > 0 && R_NR52 & enable) 
            {
            period = PERIOD(freq);
            for (i = 0, fill_idx=fill_start; 
                fill_idx != fill_end; 
                i++, fill_idx=(fill_idx+1)%AUDIO_BUFFER_SIZE)
                {
                sample = RESAMPLE(sample_count+i, period, vol);
                AUDIO_BUFFER_L[fill_idx] += sample;
                AUDIO_BUFFER_R[fill_idx] += sample;
                }
            }
#endif

#if 1
        // channel 4
        vol = (R_NR42 & NR42_INIT_VOLUME);

        // decrement length
        if ((R_NR44 & NR44_COUNTER) && (R_NR41 & NR41_SOUND_LEN))
            {
            sndlen = (R_NR41 & NR41_SOUND_LEN) + 1;
            R_NR41 = (sndlen & NR41_SOUND_LEN);
            if (sndlen & 0x40)
                {
                R_NR52 = R_NR52 & (~NR52_CH4_ON);
                }
            }    

        enable = (R_NR52 & NR52_CH4_ON);
        if (vol > 0 && R_NR52 & enable) 
            {
            for (i = 0, fill_idx=fill_start; 
                fill_idx != fill_end; 
                i++, fill_idx=(fill_idx+1)%AUDIO_BUFFER_SIZE)
                {
                sample = NOISE(sample_count+i, vol);
                AUDIO_BUFFER_L[fill_idx] += sample;
                AUDIO_BUFFER_R[fill_idx] += sample;
                }
            }
#endif
        }

#ifdef SAVE_AUDIO_DATA_RAW
    for (i = 0, fill_idx=fill_start; 
        fill_idx != fill_end; 
        i++, fill_idx=(fill_idx+1)%AUDIO_BUFFER_SIZE)
        {
        fwrite(AUDIO_BUFFER_L + fill_idx, sizeof(AUDIO_BUFFER_L[0]), 1, raw);
        }
#endif

    // set ending
    buffer_end = fill_end;
    // tracking
    audio_frame++;
    sample_count += fill_amt;

    }

/*
    Provide sound to SDL audio.
*/
void SDLFillAudio(void* udata, s16* stream, int len)
    {
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
    if (flag) printf("audio buffer underflow\n");

#ifdef SAVE_AUDIO_DATA_SDL
    fwrite(stream, sizeof(stream[0]), len, raw);
#endif
    }


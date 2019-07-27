/* Audio Support */
#include "gameboy.h"
#include "sound.h"

#include <stdio.h>
#include <math.h>

// #define SAVE_AUDIO_DATA_RAW

s16 AUDIO_BUFFER_L[AUDIO_BUFFER_SIZE];
s16 AUDIO_BUFFER_R[AUDIO_BUFFER_SIZE];


u32 audio_frame = 0;
u32 sample_count = 0;
u32 buffer_start = 0;
u32 buffer_end = 0;
u32 fill_amt = 0;
u32 fill_start = 0;
u32 fill_end = 0;
u32 fill_idx = 0;


FILE* raw = NULL;

u8 ARAM[ARAM_SIZE] = { 0x84, 0x40, 0x43, 0xAA, 0x2D, 0x78, 0x92, 0x3C, 0x60, 0x59, 0x59, 0xB0, 0x34, 0xB8, 0x2E, 0xDA };

typedef struct {
    u8  sweep_time;
    u8  sweep_dir;
    u8  sweep_shift;
    u16 sweep_freq;
    u16 timer;
} SWEEP;

typedef struct {
    u8  disabled;
    u8  envelope_volume;
    u8  envelope_dir;
    u8  envelope_sweep;
    u16 timer;
} ENVELOPE;

typedef struct {
    u16 length;
    u16 timer;
} COUNTER;

struct 
{
    SWEEP       sweep;
    ENVELOPE    envelope;
    COUNTER     counter;
    u16         freq;
} R_CHANNEL1 = {0};

struct
    {
    // real values
    u8 chan_init;
    u8 sweep_time;
    u8 sweep_dir;
    u8 sweep_shift;
    u8 wave_duty;
    u8 sound_len;
    u8 init_volume;
    u8 env_dir;
    u8 env_sweep;
    u16 freq;
    u8 counter_enable;
    // shadow values
    u8 env_enable;
    u8 sound_counter;
    u8 env_counter;
    u8 sweep_counter;
    } R_CHAN1 = {0};

struct 
{
    ENVELOPE    envelope;
    COUNTER     counter;
    u16         freq;
} R_CHANNEL2 = {0};

struct 
{
    u8          seek;
    COUNTER     counter;
} R_CHANNEL3 = {0};

struct {
    SWEEP       sweep;
    ENVELOPE    envelope;
    COUNTER     counter;
} R_CHANNEL4 = {0};

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
        case 0x10: 
            return ((R_CHAN1.sweep_time << 4) & NR10_SWEEP_TIME)
                 | ((R_CHAN1.sweep_dir << 3) & NR10_SWEEP_DECREASE)
                 | ((R_CHAN1.sweep_shift) & NR10_SWEEP_SHIFT);

        case 0x11: 
            return ((R_CHAN1.wave_duty) << 6) & NR11_WAVE_DUTY;

        case 0x12: 
            return ((R_CHAN1.init_volume << 4) & NR12_INIT_VOLUME)
                 | ((R_CHAN1.env_dir << 3) & NR12_ENV_DIR)
                 | ((R_CHAN1.env_sweep) & NR12_ENV_SWEEP);

        case 0x13: 
            return /*R_NR13*/ 0;

        case 0x14: 
            return ((R_CHAN1.counter_enable << 6) & NR14_COUNTER);

        case 0x15: return /*N/A*/ R_NR20;
        case 0x16: return R_NR21 & NR21_WAVE_DUTY;
        case 0x17: return R_NR22;
        case 0x18: return /*R_NR23*/ 0;
        case 0x19: return R_NR24 & NR24_COUNTER;
        case 0x1A: return R_NR30 & NR30_SOUND_ON;
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
    u8 n;

    // sound writes don't work if all off
    if (addr != 0x26 && !(R_NR52 & NR52_ALL_SOUND)) return;

    // write sound register / data
    switch (addr)
        {
        case 0x10: 
            R_CHAN1.sweep_time = (val & NR10_SWEEP_TIME) >> 4;
            R_CHAN1.sweep_dir = (val & NR10_SWEEP_DECREASE) >> 3;
            R_CHAN1.sweep_shift = (val & NR10_SWEEP_SHIFT);
            return;

        case 0x11: 
            R_CHAN1.wave_duty = (val & NR11_WAVE_DUTY) >> 6;
            R_CHAN1.sound_len = (val & NR11_SOUND_LEN);
            return;

        case 0x12: 
            R_CHAN1.init_volume = (val & NR12_INIT_VOLUME) >> 4;
            R_CHAN1.env_dir = (val & NR12_ENV_DIR) >> 3;
            R_CHAN1.env_sweep = (val & NR12_ENV_SWEEP);
            return;

        case 0x13: 
            R_CHAN1.freq = (R_CHAN1.freq & ~NR13_FREQ_LO)
                         | (val & NR13_FREQ_LO);
            return;

        case 0x14: 
            if (val & NR14_INIT)
                {
                R_NR52 |= NR52_CH1_ON;
                R_CHAN1.env_enable = 1;
                R_CHAN1.sound_counter = 0;
                R_CHAN1.env_counter = 0;
                R_CHAN1.sweep_counter = 0;
                R_CHAN1.chan_init = 1;
                }
            R_CHAN1.counter_enable = (val & NR14_COUNTER) ? 1 : 0;
            R_CHAN1.freq = (R_CHAN1.freq & NR13_FREQ_LO)
                         | ((val & NR14_FREQ_HI) << 8);
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
                R_CHANNEL2.envelope.disabled = 0;
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
                //printf("ch3\n");
                R_NR52 |= NR52_CH3_ON;
                R_CHANNEL3.seek = 0;
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
                R_CHANNEL4.envelope.disabled = 0;
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
    s16 svol = vol / 0x02;
    static u32 rand_state = 0xF390439F;
    rand_state = 137 * rand_state + ((rand_state / 0x08) | (rand_state << 0x18)) + 1;

    return svol * (rand_state & 0xFF - 0x80);
    }

/*
    Process GB frequency value.
*/
u32 PERIOD(u16 xfreq)
    {
    u32 fp_conversion = (0x10 * SAMPLING_RATE / AUDIO_SAMPLING_RATE);
    return (fp_conversion * (2048 - xfreq));
    }

/*
    Signal generator
*/
s16 RESAMPLE(u32 tick, u32 fp_period, u8 vol, u8 duty)
    {
    s16 svol = vol;
    s32 sample;
    u8 fp_phase, phase;
    u32 subtick;

    fp_phase = ((0x10 * tick / fp_period) % 0x80);
    phase = fp_phase / 0x10;
    subtick = ((0x10 * tick) % fp_period) * 0x80 / fp_period;

#ifdef SQUARE_WAVE
    u8 duties[4] = { 0x01, 0x81, 0x83, 0x7E };
    if (duties[duty] & (0x80 >> (subtick / 0x10)))
        {
        return 0x20 * svol;
        }
    else
        {
        return -0x20 * svol;
        }
#endif
#ifdef TRIANGLE_WAVE
        subtick = (subtick + 0x20) % 0x80;
        if (subtick > 0x40)
            {
            subtick = 0x80 - subtick;
            }
        return svol * ((s16)subtick - 0x20);
#endif
    }

// return 1 if expired
u8 SOUND_COUNTER(u8 en, u8 len, u8* count)
    {
    if (en)
        {
        if ((*count)-- == len)
            {
            return 1;
            }
        }
    return 0;
    }

// return frequency
u16 SWEEP_EFFECT(u16* freq, u8 time, u8 dir, u8 shift, u8* count)
    {
    return *freq;
    }

// return volume
u8 VOLUME_ENVELOPE(u8* init, u8 dir, u8 sweep, u8* count)
    {
    return *init << 4;
    }

// modify soundwave
void RENDER_SOUND(u16 freq, u8 vol, u8 duty)
    {
    s16 sample;
    u32 period;
    u32 i;
    if (vol > 0)
        {
        period = PERIOD(freq);
        for (i = 0, fill_idx=fill_start; 
            fill_idx != fill_end; 
            i++, fill_idx=(fill_idx+1)%AUDIO_BUFFER_SIZE)
            {
            sample = RESAMPLE(sample_count+i, period, vol, duty);
            AUDIO_BUFFER_L[fill_idx] += sample;
            AUDIO_BUFFER_R[fill_idx] += sample;
            }
        }
    }


/*
    Handle audio logic and generate sound at 256hz
*/
void AudioUpdate()
    {
    u32 audio_cycle = audio_frame;
    u16 freq; 
    u32 period; 
    u8 vol; 
    u8 env;
    u8 dir;
    u8 duty;
    u8 enable; 
    u8 disable;
    u8 sndlen; 
    s16 sample;
    u32 i; 

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
        enable = 1; //(R_NR52 & NR52_CH1_ON);

        disable = 
            SOUND_COUNTER(
                R_CHAN1.counter_enable, 
                R_CHAN1.sound_len, 
                &R_CHAN1.sound_counter
            );

        freq =
            SWEEP_EFFECT(
                &R_CHAN1.freq,
                R_CHAN1.sweep_time,
                R_CHAN1.sweep_dir,
                R_CHAN1.sweep_shift,
                &R_CHAN1.sweep_counter
            );

        vol = 
            VOLUME_ENVELOPE(
                &R_CHAN1.init_volume,
                R_CHAN1.env_dir,
                R_CHAN1.env_sweep,
                &R_CHAN1.env_counter
            );

        if (disable)
            {
            R_NR52 = R_NR52 & ~NR52_CH1_ON;
            }
        else if (enable)
            {
            RENDER_SOUND(
                freq, 
                vol, 
                R_CHAN1.wave_duty
            );
            }
#endif

#if 1
        // channel 2
        freq = (R_NR23 & NR23_FREQ_LO) | ((R_NR24 & NR24_FREQ_HI) << 8);
        vol = (R_NR22 & NR22_INIT_VOLUME);
        duty = (R_NR21 & NR21_WAVE_DUTY) >> 6;

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

        // volume envelope
        env = (R_NR22 & NR22_ENV_SWEEP);
        // 64hz update
        if (env && (audio_frame % 4) == 0)
            {
            // every 1 to 7 steps
            if (((audio_frame / 4) % env) == 0)
                {
                dir = (R_NR22 & NR22_ENV_DIR);
                // inc or dec
                if (dir)
                    {
                    vol++;
                    }
                else
                    {
                    vol--;
                    }
                // check for disable
                if (vol > 0x0F)
                    {
                    R_CHANNEL2.envelope.disabled = 1;
                    }
                else
                    {
                    // write back
                    R_NR22 = (R_NR22 & ~NR12_INIT_VOLUME) | (vol << 4);
                    }
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
                sample = RESAMPLE(sample_count+i, period, vol, duty);
                AUDIO_BUFFER_L[fill_idx] += sample;
                AUDIO_BUFFER_R[fill_idx] += sample;
                }
            }
#endif

#if 0
        freq = (R_NR33 & NR33_FREQ_LO) | ((R_NR34 & NR34_FREQ_HI) << 8);
        vol = (R_NR32 & NR32_OUT_LEVEL);

        enable = (R_NR30 & NR30_SOUND_ON) && (R_NR52 & NR52_CH3_ON);
        if (enable)
            {
            u32 seek; u8 aseek;
            u32 tick;
            for (i = 0, fill_idx=fill_start; 
                fill_idx != fill_end; 
                i++, fill_idx=(fill_idx+1)%AUDIO_BUFFER_SIZE)
                {
                // calculate sample
                period = PERIOD(freq) / 0x20; 
                period = period ? period : 1;
                seek = R_CHANNEL3.seek + tick / period;

                // break after sound length reached, if counting
                if ((R_NR34 & NR34_COUNTER) && (seek > (0x100 - R_NR31)))
                    {
                    R_NR52 = R_NR52 & (~NR52_CH3_ON);
                    break;
                    }

                if (vol)
                    {
                    aseek = seek % (2 * ARAM_SIZE);
                    // grab sample
                    u8 raw_sample = (ARAM[aseek/2] >> ((aseek % 2) ? 0 : 4)) & 0x0F;

                    // volume attenuation
                    sample = raw_sample << (10 - vol);

                    // add to sound
                    AUDIO_BUFFER_L[fill_idx] += sample;
                    AUDIO_BUFFER_R[fill_idx] += sample;
                    }
                }
            R_CHANNEL3.seek = seek;
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

        // volume envelope
        env = (R_NR42 & NR42_ENV_SWEEP) && !R_CHANNEL4.envelope.disabled;;
        // 64hz update
        if (env && (audio_frame % 4) == 0)
            {
            // every 1 to 7 steps
            if (((audio_frame / 4) % env) == 0)
                {
                dir = (R_NR42 & NR42_ENV_DIR);
                // inc or dec
                if (dir)
                    {
                    vol++;
                    }
                else
                    {
                    vol--;
                    }
                // check for disable
                if (vol > 0x0F)
                    {
                    R_CHANNEL4.envelope.disabled = 1;
                    }
                else
                    {
                    // write back
                    R_NR42 = (R_NR42 & ~NR42_INIT_VOLUME) | (vol << 4);
                    }
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


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

u8 AUDIO_RAM[AUDIO_RAM_SIZE] = { 
    0x84, 0x40, 0x43, 0xAA, 0x2D, 0x78, 0x92, 0x3C, 0x60, 0x59, 0x59, 0xB0, 0x34, 0xB8, 0x2E, 0xDA 
};


// Audio Channel 1 - Square Wave 1
struct {
    SWEEP       sweep;      /* NR10 */
    DUTY_LEN    duty_len;   /* NR11 */
    ENVELOPE    envelope;    /* NR12 */
    CHANNEL     channel;    /* NR13, NR14 */
} CH1;

// Audio Channel 2 - Square Wave 2
struct {
    DUTY_LEN    duty_len;   /* NR21 */
    ENVELOPE    envelope;    /* NR22 */
    CHANNEL     channel;    /* NR23, NR24 */
} CH2;

// Audio Channel 3 - DAC
struct {
    u8          enable;     /* NR30 */
    u8          sound_len;  /* NR31 */
    u8          out_level;  /* NR32 */
    CHANNEL     channel;    /* NR33, NR34 */
} CH3;

// Audio Channel 4 - Noise
struct {
    DUTY_LEN    len;        /* NR41 */ // no duty
    ENVELOPE    envelope;   /* NR42 */
    u8          NR43;       /* NR43 */
    CHANNEL     channel;    /* NR44 */ // no freq
} CH4;

// Sound system
struct {
    u8  NR50;
    u8  NR51;
    u8  NR52;
} SO;


u8 READ_SWEEP(SWEEP* sweep)
    {
    return (
        ((sweep->time << SWEEP_TIME_OFFS) & SWEEP_TIME_BITS) |
        ((sweep->dir << SWEEP_DIR_OFFS) & SWEEP_DIR_BIT) |
        ((sweep->shift << SWEEP_SHIFT_OFFS) & SWEEP_SHIFT_BITS)
    );
    }

void WRITE_SWEEP(SWEEP* sweep, u8 value)
    {
    sweep->time = (value & SWEEP_TIME_BITS) >> SWEEP_TIME_OFFS;
    sweep->dir = (value & SWEEP_DIR_BIT) >> SWEEP_DIR_OFFS;
    sweep->shift = (value & SWEEP_SHIFT_BITS) >> SWEEP_SHIFT_OFFS;
    }

u8 READ_DUTY_LEN(DUTY_LEN* duty_len)
    {
    return (
        ((duty_len->duty << DUTY_OFFS) & DUTY_BITS) |
        ((duty_len->len << SOUND_LEN_OFFS) & SOUND_LEN_BITS)
    );
    }

void WRITE_DUTY_LEN(DUTY_LEN* duty_len, u8 value)
    {
    duty_len->duty = (value & DUTY_BITS) >> DUTY_OFFS;
    duty_len->len = (value & SOUND_LEN_BITS) >> SOUND_LEN_OFFS;
    }

u8 READ_ENVELOPE(ENVELOPE* env)
    {
    return (
        ((env->volume << INIT_VOLUME_OFFS) & INIT_VOLUME_BITS) |
        ((env->dir << ENV_DIR_OFFS) & ENV_DIR_BIT) |
        ((env->sweep << ENV_SWEEP_OFFS) & ENV_SWEEP_BITS)
    );
    }

void WRITE_ENVELOPE(ENVELOPE* env, u8 value)
    {
    env->volume = (value & INIT_VOLUME_BITS) >> INIT_VOLUME_OFFS;
    env->dir = (value & ENV_DIR_BIT) >> ENV_DIR_OFFS;
    env->sweep = (value & ENV_SWEEP_BITS) >> ENV_SWEEP_OFFS;
    }

u8 READ_CHANNEL_LO(CHANNEL* channel)
    {
    return (channel->freq & FREQ_LO_MASK);
    }

void WRITE_CHANNEL_LO(CHANNEL* channel, u8 value)
    {
    channel->freq = (channel->freq & FREQ_HI_MASK) | (value & FREQ_LO_MASK);
    }

u8 READ_CHANNEL_HI(CHANNEL* channel)
    {
    return (
        (channel->counter << COUNTER_OFFS) & COUNTER_BIT
    );
    }

void WRITE_CHANNEL_HI(CHANNEL* channel, u8 value)
    {
    channel->initset = (value & INIT_BIT);
    channel->counter = (value & COUNTER_BIT) >> COUNTER_OFFS;
    channel->freq = (channel->freq & FREQ_LO_MASK) | ((value & FREQ_HI_BITS) << 8);
    }


/*
    Handle reads from sound controller.
*/
u8 AUDIO_READ(u8 addr)
    {
    // sound reads don't work at all if off
    if (!(SO.NR52 & NR52_ALL_SOUND)) return 0;

    // read sound registers / data
    switch (addr)
        {
        /* NR10 - Ch1 Sweep */
        case 0x10: return READ_SWEEP(&CH1.sweep);

        /* NR11 - Ch1 Duty / Len */
        case 0x11: return READ_DUTY_LEN(&CH1.duty_len);

        /* NR12 - Ch1 Envelope */
        case 0x12: return READ_ENVELOPE(&CH1.envelope);

        /* NR13 - Ch1 Lo Freq */
        case 0x13: return READ_CHANNEL_LO(&CH1.channel);

        /* NR14 - CH1 Init, Counter, Hi Freq */
        case 0x14: return READ_CHANNEL_HI(&CH1.channel);

        /* NR20 - Not Used */
        case 0x15: return 0;

        /* NR21 - Ch2 Duty / Len */
        case 0x16: return READ_DUTY_LEN(&CH2.duty_len);

        /* NR22 - Ch2 Envelope */
        case 0x17: return READ_ENVELOPE(&CH2.envelope);

        /* NR23 - Ch2 Lo Freq */
        case 0x18: return READ_CHANNEL_LO(&CH2.channel);

        /* NR24 - Ch2 Init, Counter, Hi Freq */
        case 0x19: return READ_CHANNEL_HI(&CH2.channel);

        /* NR30 - Ch3 Sound On */
        case 0x1A: 
            return (CH3.enable << NR30_SOUND_ON_OFFS) & NR30_SOUND_ON_BIT;

        /* NR31 - Ch3 Sound Len */
        case 0x1B: return CH3.sound_len;

        /* NR32 - Ch3 Volume Out Level */
        case 0x1C: 
            return (CH3.out_level << NR32_OUT_LEVEL_OFFS) & NR32_OUT_LEVEL_BITS;

        /* NR33 - Ch3 Lo Freq */
        case 0x1D: return READ_CHANNEL_LO(&CH3.channel);

        /* NR34 - Ch3 Init, Counter, Hi Freq */
        case 0x1E: return READ_CHANNEL_HI(&CH3.channel);

        /* NR40 - Not Used */
        case 0x1F: return 0;

        /* NR41 - Ch4 Len Register */
        case 0x20: return READ_DUTY_LEN(&CH4.len);

        /* NR42 - Ch4 Envelope */
        case 0x21: return READ_ENVELOPE(&CH4.envelope);

        /* NR43 - Ch4 Clock, Step, Ratio */
        case 0x22: return CH4.NR43;

        /* NR44 - Ch4 Init, Counter */
        case 0x23: return READ_CHANNEL_HI(&CH4.channel);

        /* NR50 */
        case 0x24: return SO.NR50;

        /* NR51 */
        case 0x25: return SO.NR51;

        /* NR52 */
        case 0x26: return SO.NR52;
        
        // wave pattern RAM
        case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37: 
        case 0x38: case 0x39: case 0x3A: case 0x3B: case 0x3C: case 0x3D: case 0x3E: case 0x3F: 
            return AUDIO_RAM[(addr - 0x30)];
        }
    return 0;
    }

/*
    Handle writes to sound controller.
*/
void AUDIO_WRITE(u8 addr, u8 val)
    {
    // sound writes don't work if all off
    if (addr != 0x26 && !(SO.NR52 & NR52_ALL_SOUND)) return;
switch (addr)
        {
        /* NR10 - Ch1 Sweep */
        case 0x10: 
            WRITE_SWEEP(&CH1.sweep, val);
            return;

        /* NR11 - Ch1 Duty / Len */
        case 0x11: 
            WRITE_DUTY_LEN(&CH1.duty_len, val);
            return;

        /* NR12 - Ch1 Envelope */
        case 0x12: 
            WRITE_ENVELOPE(&CH1.envelope, val);
            return;

        /* NR13 - Ch1 Lo Freq */
        case 0x13: 
            WRITE_CHANNEL_LO(&CH1.channel, val);
            return;

        /* NR14 - CH1 Init, Counter, Hi Freq */
        case 0x14: 
            WRITE_CHANNEL_HI(&CH1.channel, val);
            return;

        /* NR20 - Not Used */
        case 0x15: return;

        /* NR21 - Ch2 Duty / Len */
        case 0x16: 
            WRITE_DUTY_LEN(&CH2.duty_len, val);
            return;

        /* NR22 - Ch2 Envelope */
        case 0x17: 
            WRITE_ENVELOPE(&CH2.envelope, val);
            return;

        /* NR23 - Ch2 Lo Freq */
        case 0x18:
            WRITE_CHANNEL_LO(&CH2.channel, val);
            return;

        /* NR24 - Ch2 Init, Counter, Hi Freq */
        case 0x19: 
            WRITE_CHANNEL_HI(&CH2.channel,val);
            return;

        /* NR30 - Ch3 Sound On */
        case 0x1A: 
            CH3.enable = (val & NR30_SOUND_ON_BIT) >> NR30_SOUND_ON_OFFS;
            return;

        /* NR31 - Ch3 Sound Len */
        case 0x1B: 
            CH3.sound_len = val;
            return;

        /* NR32 - Ch3 Volume Out Level */
        case 0x1C: 
            CH3.out_level = (val >> NR32_OUT_LEVEL_OFFS) & NR32_OUT_LEVEL_BITS;
            return;

        /* NR33 - Ch3 Lo Freq */
        case 0x1D: 
            WRITE_CHANNEL_LO(&CH3.channel, val);
            return;

        /* NR34 - Ch3 Init, Counter, Hi Freq */
        case 0x1E: 
            WRITE_CHANNEL_HI(&CH3.channel, val);
            return;

        /* NR40 - Not Used */
        case 0x1F: return;

        /* NR41 - Ch4 Len Register */
        case 0x20: 
            WRITE_DUTY_LEN(&CH4.len, val);
            return;

        /* NR42 - Ch4 Envelope */
        case 0x21: 
            WRITE_ENVELOPE(&CH4.envelope, val);
            return;

        /* NR43 - Ch4 Clock, Step, Ratio */
        case 0x22: 
            CH4.NR43 = val;
            return;

        /* NR44 - Ch4 Init, Counter */
        case 0x23:  
            WRITE_CHANNEL_HI(&CH4.channel, val);
            return;

        /* NR50 */
        case 0x24: 
            SO.NR50 = val;
            return;

        /* NR51 */
        case 0x25: 
            SO.NR51 = val;
            return;

        /* NR52 */
        case 0x26: 
            SO.NR52 = (val & NR52_ALL_SOUND) | (SO.NR52 & NR52_STATUS);
            if (!(val & NR52_ALL_SOUND))
                {
                SO.NR52 = 0;
                }
            return;
        
        // wave pattern RAM
        case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37: 
        case 0x38: case 0x39: case 0x3A: case 0x3B: case 0x3C: case 0x3D: case 0x3E: case 0x3F: 
            AUDIO_RAM[(addr - 0x30)] = val;
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
    if (SO.NR52 & NR52_ALL_SOUND)
        {

        if (CH1.channel.initset)
            {
            RENDER_SOUND(
                CH1.channel.freq,
                CH1.envelope.volume,
                CH1.duty_len.duty
            );
            }

        if (CH2.channel.initset)
            {
            RENDER_SOUND(
                CH2.channel.freq,
                CH2.envelope.volume,
                CH2.duty_len.duty
            );
            }
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


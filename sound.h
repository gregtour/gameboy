#ifndef _SOUND_H
#define _SOUND_H

#include "SDL_audio.h"

#define SQUARE_WAVE
//#define TRIANGLE_WAVE

#define BIT(x)      (1 << (x))

#define AUDIO_RAM_SIZE			16

// No sound support yet
#define SAMPLING_RATE		48000
#define SAMPLING_SIZE		1024
#define AUDIO_CHANNELS 		2

#define AUDIO_SAMPLING_RATE	0x20000
#define AUDIO_BUFFER_SIZE	(8*SAMPLING_SIZE)
#define AUDIO_CYCLES		15625
#define AUDIO_FILL			(SAMPLING_RATE / (CPU_CLOCKSPEED / AUDIO_CYCLES))


// sweep register
// NR10 sweep register
typedef struct {
    u8  time;
    u8  dir;
    u8  shift;
    u16 freq;
    u16 timer;
} SWEEP;

#define SWEEP_TIME_BITS 	(0x70)
#define SWEEP_TIME_OFFS		(4)
#define SWEEP_DIR_BIT 		(0x08)
#define SWEEP_DIR_OFFS 		(3)
#define SWEEP_SHIFT_BITS	(0x07)
#define SWEEP_SHIFT_OFFS 	(0)

// duty / len register
// NR11, NR21, NR41 len register
typedef struct {
    u8 duty;
    u8 len;
} DUTY_LEN;

#define DUTY_BITS			(0xC0)
#define DUTY_OFFS 			(6)
#define SOUND_LEN_BITS 		(0x3F)
#define SOUND_LEN_OFFS 		(0)

// NR12, NR22, NR42 envelope register
typedef struct {
    u8  disabled;
    u8  volume;
    u8  dir;
    u8  sweep;
    u16 timer;
} ENVELOPE;

#define INIT_VOLUME_BITS 	(0xF0)
#define INIT_VOLUME_OFFS 	(4)
#define ENV_DIR_BIT			(0x08)
#define ENV_DIR_OFFS 		(3)
#define ENV_SWEEP_BITS 		(0x07)
#define ENV_SWEEP_OFFS 		(0)

// NR13 & 14, NR23 & 24, NR33 & 34, NR44: init, counter, freq
typedef struct {
	u8 	initset;
	u8  counter;
    u16 freq;
    u16 timer;
} CHANNEL;

#define FREQ_LO_MASK 		(0x00FF)
#define FREQ_HI_MASK 		(0xFF00)
#define INIT_BIT 			(0x80)
#define COUNTER_BIT 		(0x40)
#define COUNTER_OFFS 		(6)
#define FREQ_HI_BITS 		(0x07)

// NR30, NR31
#define NR30_SOUND_ON_BIT	(0x80)
#define NR30_SOUND_ON_OFFS 	(7)
#define NR31_SOUND_LEN_BITS	(0xFF)
#define NR32_OUT_LEVEL_BITS	(0x60)
#define NR32_OUT_LEVEL_OFFS	(5)

// NR43
#define NR43_SHIFT_CLOCK_BITS	(0xF0)
#define NR43_SHIFT_CLOCK_OFFS 	(4)
#define NR43_COUNTER_STEP_BITS 	(0x08)
#define NR43_COUNTER_STEP_OFFS 	(3)
#define NR43_DIV_RATIO_BITS 	(0x03)
#define NR43_DIV_RATIO_OFFS 	(0)

/* Sound Control */
#define NR50_SO2_ENABLE     (0x80)
#define NR50_SO2_LEVEL      (0x70)
#define NR50_SO1_ENABLE     (0x08)
#define NR50_SO1_LEVEL      (0x07)

#define NR51_CH4_SO2        (0x80)
#define NR51_CH3_SO2        (0x40)
#define NR51_CH2_SO2        (0x20)
#define NR51_CH1_SO2        (0x10)
#define NR51_CH4_SO1        (0x08)
#define NR51_CH3_SO1        (0x04)
#define NR51_CH2_SO1        (0x02)
#define NR51_CH1_SO1        (0x01)

#define NR52_ALL_SOUND      (0x80)
#define NR52_STATUS         (0x0F)
#define NR52_CH4_ON         (0x08)
#define NR52_CH3_ON         (0x04)
#define NR52_CH2_ON         (0x02)
#define NR52_CH1_ON         (0x01)

void SDLAudioStart();

void AudioUpdate();

void AUDIO_WRITE(u8 addr, u8 val);
u8   AUDIO_READ(u8 addr);

#endif // _SOUND_H


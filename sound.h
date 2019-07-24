#ifndef _SOUND_H
#define _SOUND_H

#include "SDL_audio.h"

#define BIT(x)      (1 << (x))

// No sound support yet
#define SAMPLING_RATE		48000
#define SAMPLING_SIZE		2048

#define AUDIO_BUFFER_SIZE	4096
#define AUDIO_CYCLES		15625

/* Channel 1 */
#define NR10_SWEEP_TIME     (0x70)
#define NR10_SWEEP_DECREASE (0x08)
#define NR10_SWEEP_SHIFT    (0x07)
#define NR11_WAVE_DUTY      (0xC0)
#define NR11_SOUND_LEN      (0x3F)
#define NR12_INIT_VOLUME    (0xF0)
#define NR12_ENV_DIR        (0x08)
#define NR12_ENV_SWEEP      (0x07)
#define NR13_FREQ_LO        (0xFF)
#define NR14_INIT           (0x80)
#define NR14_COUNTER        (0x40)
#define NR14_FREQ_HI        (0x07)

/* Channel 2 */
#define NR21_WAVE_DUTY      (0xC0)
#define NR21_SOUND_LEN      (0x3F)
#define NR22_INIT_VOLUME    (0xF0)
#define NR22_ENV_DIR        (0x08)
#define NR22_ENV_SWEEP      (0x07)
#define NR23_FREQ_LO        (0xFF)
#define NR24_INIT           (0x80)
#define NR24_COUNTER        (0x40)
#define NR24_FREQ_HI        (0x07)

/* Channel 3 */
#define NR30_SOUND_ON       (0x80)
#define NR31_SOUND_LEN      (0xFF)
#define NR32_OUT_LEVEL      (0x60)
#define NR33_FREQ_LO        (0xFF)
#define NR34_INIT           (0x80)
#define NR34_COUNTER        (0x40)
#define NR34_FREQ_HI        (0x07)

/* Channel 4 */
#define NR41_SOUND_LEN      (0x3F)
#define NR42_INIT_VOLUME    (0xF0)
#define NR42_ENV_DIR        (0x08)
#define NR42_ENV_SWEEP      (0x07)
#define NR43_SHIFT_CLOCK    (0xF0)
#define NR43_COUNTER_STEP   (0x08)
#define NR43_DIV_RATIO      (0x03)
#define NR44_INIT           (0x80)
#define NR44_COUNTER        (0x40)

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

// cpu sound registers
extern u8 R_NR10;
extern u8 R_NR11;
extern u8 R_NR12;
extern u8 R_NR13;
extern u8 R_NR14;
extern u8 R_NR20;
extern u8 R_NR21;
extern u8 R_NR22;
extern u8 R_NR23;
extern u8 R_NR24;
extern u8 R_NR30;
extern u8 R_NR31;
extern u8 R_NR32;
extern u8 R_NR33;
extern u8 R_NR34;
extern u8 R_NR40;
extern u8 R_NR41;
extern u8 R_NR42;
extern u8 R_NR43;
extern u8 R_NR44;
extern u8 R_NR50;
extern u8 R_NR51;
extern u8 R_NR52;

void SDLAudioStart();

void AudioUpdate();

void AUDIO_WRITE(u8 addr, u8 val);
u8   AUDIO_READ(u8 addr);

#endif // _SOUND_H


/*
 * record.h
 *
 *  Created on: Sep 13, 2022
 *      Author: HP
 */

#ifndef INC_RECORD_H_
#define INC_RECORD_H_

#include "stm32746g_discovery_audio.h"
#include "ff.h"
#include "serial.h"
#include "fatfs.h"
#include "string.h"
#include "audio.h"

#define AUDIO_IN_PCM_BUFFER_SIZE                   4*2304 /* buffer size in half-word */

#define REC_WAVE_NAME "Wave.wav"
#define DEFAULT_TIME_REC                      20  /* Recording time in second (default: 20s) */
#define REC_SAMPLE_LENGTH   (DEFAULT_TIME_REC * DEFAULT_AUDIO_IN_FREQ * 1 * 2)

typedef enum {
  BUFFER_EMPTY = 0,
  BUFFER_FULL,
}WR_BUFFER_StateTypeDef;

typedef struct {
  uint16_t pcm_buff[AUDIO_IN_PCM_BUFFER_SIZE*2];
  uint32_t pcm_ptr;
  WR_BUFFER_StateTypeDef wr_state;
  uint32_t offset;
  uint32_t fptr;
}AUDIO_IN_BufferTypeDef;

AUDIO_ErrorTypeDef recordStart(void);
AUDIO_ErrorTypeDef recordProcess(void);
AUDIO_ErrorTypeDef recordStop(void);

#endif /* INC_RECORD_H_ */

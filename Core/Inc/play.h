/*
 * play.h
 *
 *  Created on: Sep 15, 2022
 *      Author: HP
 */

#ifndef INC_PLAY_H_
#define INC_PLAY_H_

#include "stm32746g_discovery_audio.h"
#include "stdio.h"
#include "fatfs.h"
#include "string.h"
#include "serial.h"
#include "audio.h"

#define AUDIO_OUT_BUFFER_SIZE	8192

typedef enum {
  BUFFER_OFFSET_NONE = 0,
  BUFFER_OFFSET_HALF,
  BUFFER_OFFSET_FULL,
}BUFFER_StateTypeDef;

/* Audio buffer control struct */
typedef struct {
  uint8_t buff[AUDIO_OUT_BUFFER_SIZE*2];
  BUFFER_StateTypeDef state;
  uint32_t fptr;
}AUDIO_OUT_BufferTypeDef;

AUDIO_ErrorTypeDef playStart(int index);
AUDIO_ErrorTypeDef playProcess(void);
AUDIO_ErrorTypeDef playStop(void);

#endif /* INC_PLAY_H_ */

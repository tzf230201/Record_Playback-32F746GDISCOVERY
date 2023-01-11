/*
 * role.h
 *
 *  Created on: Sep 17, 2022
 *      Author: HP
 */

#ifndef INC_ROLE_H_
#define INC_ROLE_H_

#include "serial.h"
#include "record.h"
#include "play.h"

typedef enum {
  RECORD_START = 0,
  RECORD_PROCESS,
  RECORD_STOP,
  PLAY_START,
  PLAY_PROCESS,
  PLAY_STOP,
  AUDIO_GRAPH
}ROLE_TypeDef;

void roleInit(void);
void roleNode(void);

#endif /* INC_ROLE_H_ */

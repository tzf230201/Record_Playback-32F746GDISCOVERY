/*
 * role.c
 *
 *  Created on: Sep 17, 2022
 *      Author: HP
 */

#include "role.h"
#include "stm32746g_discovery_lcd.h"
#include "stm32746g_discovery_ts.h"

ROLE_TypeDef role = RECORD_START;

#define SONG_NUMBER 0

static void LCD_Config(void)
{
  /* LCD Initialization */
  BSP_LCD_Init();

  /* LCD Initialization */
  BSP_LCD_LayerDefaultInit(0, LCD_FB_START_ADDRESS);
  BSP_LCD_LayerDefaultInit(1, LCD_FB_START_ADDRESS+(BSP_LCD_GetXSize()*BSP_LCD_GetYSize()*4));

  /* Enable the LCD */
  BSP_LCD_DisplayOn();

  /* Select the LCD Background Layer  */
  BSP_LCD_SelectLayer(0);

  /* Clear the Background Layer */
  BSP_LCD_Clear(LCD_COLOR_BLACK);

  /* Select the LCD Foreground Layer  */
  BSP_LCD_SelectLayer(1);

  /* Clear the Foreground Layer */
  BSP_LCD_Clear(LCD_COLOR_BLACK);
  BSP_LCD_SetBackColor(LCD_COLOR_BLACK);
  /* Configure the transparency for foreground and background :
     Increase the transparency */
}


void roleInit()
{
	FRESULT res; /* FatFs function common result code */


	/*init library serial*/
	serialInit(&vcp,USART1,usart1_buffer, sizeof(usart1_buffer));

	serialPrintln(&vcp,"mount logical drive and create a FAT volume");
	/*Mount a logical drive*/
	res = f_mount(&SDFatFS, (TCHAR const*)SDPath, 0);
	if(res != FR_OK)
	{
	  serialPrintln(&vcp,"error mount, code error : %d",res);
	}
//	else
//	{
//		/*create a FAT volume*/
//		res = f_mkfs((TCHAR const*)SDPath, FM_ANY, 0, rtext, sizeof(rtext));
//		if(res != FR_OK)
//		{
//			serialPrintln(&vcp,"error mkfs, code error : %d",res);
//		}
//	}

	LCD_Config();
//	BSP_TS_Init(480,272);
	HAL_Delay(500);

}


void roleNode()
{
	char buf[25];
	switch(role)
	{
	case RECORD_START:
		recordStart();
		BSP_LCD_Clear(LCD_COLOR_BLACK);
		BSP_LCD_SetTextColor(LCD_COLOR_RED);
		sprintf(buf, "Recording..");
		BSP_LCD_DisplayStringAt(0,150,(uint8_t*)buf,LEFT_MODE);
		role = RECORD_PROCESS;
		break;
	case RECORD_PROCESS:
		if(recordProcess() == AUDIO_ERROR_EOF)
			role = RECORD_STOP;
		break;
	case RECORD_STOP:
		recordStop();
		role = PLAY_START;
		break;
	case PLAY_START:
		BSP_LCD_Clear(LCD_COLOR_BLACK);
		BSP_LCD_SetTextColor(LCD_COLOR_GREEN);
		sprintf(buf, "Play..");
		BSP_LCD_DisplayStringAt(0,150,(uint8_t*)buf,LEFT_MODE);
		playStart(SONG_NUMBER);
		role = PLAY_PROCESS;
		break;
	case PLAY_PROCESS:
		if(playProcess() == AUDIO_ERROR_EOF)
			role = PLAY_STOP;
		break;
	case PLAY_STOP:
		playStop();
		role = AUDIO_GRAPH;
		break;
	case AUDIO_GRAPH:
		BSP_LCD_SetTextColor(LCD_COLOR_GREEN);
		BSP_LCD_DrawHLine(0, 136, 480);
		break;
	}
}

/*
 * record.c
 *
 *  Created on: Sep 15, 2022
 *      Author: HP
 */

#include "play.h"

static AUDIO_OUT_BufferTypeDef  BufferCtl;
static int16_t FilePos = 0;
static __IO uint32_t uwVolume = 70;

int tes1,tes2;

AUDIO_PLAYBACK_StateTypeDef AudioState;

WAVE_FormatTypeDef WaveReadFormat;
FILELIST_FileTypeDef FileList;

/* Private function prototypes -----------------------------------------------*/
static AUDIO_ErrorTypeDef GetFileInfo(uint16_t file_idx, WAVE_FormatTypeDef *info);
static uint8_t PlayerInit(uint32_t AudioFreq);

FILELIST_FileTypeDef FileList;
uint16_t NumObs = 0;

/**
  * @brief  Copies disk content in the explorer list.
  * @param  None
  * @retval Operation result
  */
FRESULT AUDIO_StorageParse(void)
{
  FRESULT res = FR_OK;
  FILINFO fno;
  DIR dir;
  char *fn;

  res = f_opendir(&dir, SDPath);
  FileList.ptr = 0;

  if(res == FR_OK)
  {
    while(BSP_SD_IsDetected())
    {
      res = f_readdir(&dir, &fno);
      if(res != FR_OK || fno.fname[0] == 0)
      {
        break;
      }
      if(fno.fname[0] == '.')
      {
        continue;
      }
      fn = fno.fname;

      if(FileList.ptr < FILEMGR_LIST_DEPDTH)
      {
        if((fno.fattrib & AM_DIR) == 0)
        {
          if((strstr(fn, "wav")) || (strstr(fn, "WAV")))
          {
            strncpy((char *)FileList.file[FileList.ptr].name, (char *)fn, FILEMGR_FILE_NAME_SIZE);
            FileList.file[FileList.ptr].type = FILETYPE_FILE;
            FileList.ptr++;
          }
        }
      }
    }
  }
  else
  {
	  serialPrintln(&vcp,  "cannot open dir : %d",res);

  }
  NumObs = FileList.ptr;
  serialPrintln(&vcp,"NumbObs : %d",NumObs);
  f_closedir(&dir);
  return res;
}


/**
  * @brief  Shows audio file (*.wav) on the root
  * @param  None
  * @retval None
  */
uint8_t AUDIO_ShowWavFiles(void)
{
  if(AUDIO_StorageParse() ==  FR_OK)
  {
    if(FileList.ptr > 0)
    {
      return 0;
    }
    return 1;
  }
  return 2;
}

/**
  * @brief  Gets Wav Object Number.
  * @param  None
  * @retval None
  */
uint16_t AUDIO_GetWavObjectNumber(void)
{
  return NumObs;
}

AUDIO_ErrorTypeDef AUDIO_PLAYER_Init(void)
{
  if(BSP_AUDIO_OUT_Init(OUTPUT_DEVICE_AUTO, uwVolume, AUDIO_FREQUENCY_48K) == 0)
  {
    return AUDIO_ERROR_NONE;
  }
  else
  {
    return AUDIO_ERROR_IO;
  }
}

/**
  * @brief  Initializes the Wave player.
  * @param  AudioFreq: Audio sampling frequency
  * @retval None
  */
static uint8_t PlayerInit(uint32_t AudioFreq)
{
  /* Initialize the Audio codec and all related peripherals (I2S, I2C, IOExpander, IOs...) */
  if(BSP_AUDIO_OUT_Init(OUTPUT_DEVICE_BOTH, uwVolume, AudioFreq) != 0)
  {
    return 1;
  }
  else
  {
    BSP_AUDIO_OUT_SetAudioFrameSlot(CODEC_AUDIOFRAME_SLOT_02);
    return 0;
  }
}

/**
  * @brief  Gets the file info.
  * @param  file_idx: File index
  * @param  info: Pointer to WAV file info
  * @retval Audio error
  */
static AUDIO_ErrorTypeDef GetFileInfo(uint16_t file_idx, WAVE_FormatTypeDef *info)
{
  uint32_t bytesread;
  uint32_t duration;
  uint8_t str[FILEMGR_FILE_NAME_SIZE + 20];

  if(f_open(&SDFile, (char *)FileList.file[file_idx].name, FA_OPEN_EXISTING | FA_READ) == FR_OK)
  {
    /* Fill the buffer to Send */
    if(f_read(&SDFile, info, sizeof(WaveReadFormat), (void *)&bytesread) == FR_OK)
    {

      return AUDIO_ERROR_NONE;
    }
    f_close(&SDFile);
  }
  return AUDIO_ERROR_IO;
}


AUDIO_ErrorTypeDef AUDIO_PLAYER_Start(uint8_t idx)
{
  uint32_t bytesread;

  f_close(&SDFile);
  if(AUDIO_GetWavObjectNumber() > idx)
  {

	GetFileInfo(idx, &WaveReadFormat);


    /*Adjust the Audio frequency */
    PlayerInit(WaveReadFormat.SampleRate);

    BufferCtl.state = BUFFER_OFFSET_NONE;

    /* Get Data from USB Flash Disk */
    f_lseek(&SDFile, 0);

    /* Fill whole buffer at first time */
    if(f_read(&SDFile,
              &BufferCtl.buff[0],
              AUDIO_OUT_BUFFER_SIZE,
              (void *)&bytesread) == FR_OK)
    {
        if(bytesread != 0)
        {
          BSP_AUDIO_OUT_Play((uint16_t*)&BufferCtl.buff[0], AUDIO_OUT_BUFFER_SIZE/2);
          BufferCtl.fptr = bytesread;
          return AUDIO_ERROR_NONE;
        }
    }
  }
  return AUDIO_ERROR_IO;
}



AUDIO_ErrorTypeDef playStart(int index)
{
	int res;
	serialPrintln(&vcp,"init play");
	res = f_mount(&SDFatFS, (TCHAR const*)SDPath, 0);
	if(res != FR_OK)
	{
	  serialPrintln(&vcp,"error mount, code error : %d",res);
	}

	AUDIO_ShowWavFiles();

	res = AUDIO_PLAYER_Start(index);
	if(res != AUDIO_ERROR_NONE)
	{
	  serialPrintln(&vcp,"error start audio : %d",res);
	}
	else
	{
		serialPrintln(&vcp,"audio start gaes");
	}

	return (AUDIO_ERROR_NONE);
}

AUDIO_ErrorTypeDef playProcess(void)
{
  uint32_t bytesread, elapsed_time;
  AUDIO_ErrorTypeDef audio_error = AUDIO_ERROR_NONE;
  static uint32_t prev_elapsed_time = 0xFFFFFFFF;


    if(BufferCtl.fptr >= WaveReadFormat.FileSize)
    {
      BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
      audio_error = AUDIO_ERROR_EOF;
    }

    if(BufferCtl.state == BUFFER_OFFSET_HALF)
    {

      if(f_read(&SDFile,
                &BufferCtl.buff[0],
                AUDIO_OUT_BUFFER_SIZE/2,
                (void *)&bytesread) != FR_OK)
      {
        BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
        return AUDIO_ERROR_IO;
      }
      BufferCtl.state = BUFFER_OFFSET_NONE;
      BufferCtl.fptr += bytesread;
    }

    if(BufferCtl.state == BUFFER_OFFSET_FULL)
    {

      if(f_read(&SDFile,
                &BufferCtl.buff[AUDIO_OUT_BUFFER_SIZE/2],
                AUDIO_OUT_BUFFER_SIZE/2,
                (void *)&bytesread) != FR_OK)
      {
        BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
        return AUDIO_ERROR_IO;
      }

      BufferCtl.state = BUFFER_OFFSET_NONE;
      BufferCtl.fptr += bytesread;
    }

    /* Display elapsed time */
    elapsed_time = BufferCtl.fptr / WaveReadFormat.ByteRate;
    if(prev_elapsed_time != elapsed_time)
    {
      prev_elapsed_time = elapsed_time;
    }
  return audio_error;
}


void BSP_AUDIO_OUT_TransferComplete_CallBack(void)
{
    BufferCtl.state = BUFFER_OFFSET_FULL;
}

void BSP_AUDIO_OUT_HalfTransfer_CallBack(void)
{
    BufferCtl.state = BUFFER_OFFSET_HALF;
}

AUDIO_ErrorTypeDef playStop(void)
{
  AudioState = AUDIO_STATE_STOP;
  FilePos = 0;

  BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
  f_close(&SDFile);

  serialPrintln(&vcp,"selesai play");
  return AUDIO_ERROR_NONE;
}

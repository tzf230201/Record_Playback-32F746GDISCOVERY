/*
 * record.c
 *
 *  Created on: Sep 13, 2022
 *      Author: HP
 */

#include "record.h"


#define BUF_SIZE AUDIO_IN_PCM_BUFFER_SIZE

uint16_t buffer[BUF_SIZE];
uint16_t buffer_size = BUF_SIZE;

uint8_t pHeaderBuff[44];

static AUDIO_IN_BufferTypeDef  BufferCtl;
static __IO uint32_t uwVolume = 100;
WAVE_FormatTypeDef WaveFormat;

/*******************************************************************************
                            Static Functions
*******************************************************************************/

/**
  * @brief  Initialize the wave header file
  * @param  pHeader: Header Buffer to be filled
  * @param  pWaveFormatStruct: Pointer to the wave structure to be filled.
  * @retval 0 if passed, !0 if failed.
  */
static uint32_t WavProcess_HeaderInit(uint8_t* pHeader, WAVE_FormatTypeDef* pWaveFormatStruct)
{
  /* Write chunkID, must be 'RIFF'  ------------------------------------------*/
  pHeader[0] = 'R';
  pHeader[1] = 'I';
  pHeader[2] = 'F';
  pHeader[3] = 'F';

  /* Write the file length ---------------------------------------------------*/
  /* The sampling time: this value will be written back at the end of the
     recording operation.  Example: 661500 Btyes = 0x000A17FC, byte[7]=0x00, byte[4]=0xFC */
  pHeader[4] = 0x00;
  pHeader[5] = 0x4C;
  pHeader[6] = 0x1D;
  pHeader[7] = 0x00;
  /* Write the file format, must be 'WAVE' -----------------------------------*/
  pHeader[8]  = 'W';
  pHeader[9]  = 'A';
  pHeader[10] = 'V';
  pHeader[11] = 'E';

  /* Write the format chunk, must be'fmt ' -----------------------------------*/
  pHeader[12]  = 'f';
  pHeader[13]  = 'm';
  pHeader[14]  = 't';
  pHeader[15]  = ' ';

  /* Write the length of the 'fmt' data, must be 0x10 ------------------------*/
  pHeader[16]  = 0x10;
  pHeader[17]  = 0x00;
  pHeader[18]  = 0x00;
  pHeader[19]  = 0x00;

  /* Write the audio format, must be 0x01 (PCM) ------------------------------*/
  pHeader[20]  = 0x01;
  pHeader[21]  = 0x00;

  /* Write the number of channels, ie. 0x01 (Mono) ---------------------------*/
  pHeader[22]  = pWaveFormatStruct->NbrChannels;
  pHeader[23]  = 0x00;

  /* Write the Sample Rate in Hz ---------------------------------------------*/
  /* Write Little Endian ie. 8000 = 0x00001F40 => byte[24]=0x40, byte[27]=0x00*/
  pHeader[24]  = (uint8_t)((pWaveFormatStruct->SampleRate & 0xFF));
  pHeader[25]  = (uint8_t)((pWaveFormatStruct->SampleRate >> 8) & 0xFF);
  pHeader[26]  = (uint8_t)((pWaveFormatStruct->SampleRate >> 16) & 0xFF);
  pHeader[27]  = (uint8_t)((pWaveFormatStruct->SampleRate >> 24) & 0xFF);

  /* Write the Byte Rate -----------------------------------------------------*/
  pHeader[28]  = (uint8_t)((pWaveFormatStruct->ByteRate & 0xFF));
  pHeader[29]  = (uint8_t)((pWaveFormatStruct->ByteRate >> 8) & 0xFF);
  pHeader[30]  = (uint8_t)((pWaveFormatStruct->ByteRate >> 16) & 0xFF);
  pHeader[31]  = (uint8_t)((pWaveFormatStruct->ByteRate >> 24) & 0xFF);

  /* Write the block alignment -----------------------------------------------*/
  pHeader[32]  = pWaveFormatStruct->BlockAlign;
  pHeader[33]  = 0x00;

  /* Write the number of bits per sample -------------------------------------*/
  pHeader[34]  = pWaveFormatStruct->BitPerSample;
  pHeader[35]  = 0x00;

  /* Write the Data chunk, must be 'data' ------------------------------------*/
  pHeader[36]  = 'd';
  pHeader[37]  = 'a';
  pHeader[38]  = 't';
  pHeader[39]  = 'a';

  /* Write the number of sample data -----------------------------------------*/
  /* This variable will be written back at the end of the recording operation */
  pHeader[40]  = 0x00;
  pHeader[41]  = 0x4C;
  pHeader[42]  = 0x1D;
  pHeader[43]  = 0x00;

  /* Return 0 if all operations are OK */
  return 0;
}

/**
  * @brief  Encoder initialization.
  * @param  Freq: Sampling frequency.
  * @param  pHeader: Pointer to the WAV file header to be written.
  * @retval 0 if success, !0 else.
  */
static uint32_t WavProcess_EncInit(uint32_t Freq, uint8_t *pHeader)
{
  /* Initialize the encoder structure */
  WaveFormat.SampleRate = Freq;        /* Audio sampling frequency */
  WaveFormat.NbrChannels = 1;          /* Number of channels: 1:Mono or 2:Stereo */
  WaveFormat.BitPerSample = 16;        /* Number of bits per sample (16, 24 or 32) */
  WaveFormat.FileSize = 0x001D4C00;    /* Total length of useful audio data (payload) */
  WaveFormat.SubChunk1Size = 44;       /* The file header chunk size */
  WaveFormat.ByteRate = (WaveFormat.SampleRate * \
                        (WaveFormat.BitPerSample/8) * \
                         WaveFormat.NbrChannels);     /* Number of bytes per second  (sample rate * block align)  */
  WaveFormat.BlockAlign = WaveFormat.NbrChannels * \
                         (WaveFormat.BitPerSample/8); /* channels * bits/sample / 8 */

  /* Parse the wav file header and extract required information */
  if(WavProcess_HeaderInit(pHeader, &WaveFormat))
  {
    return 1;
  }
  return 0;
}



/**
  * @brief  Initialize the wave header file
  * @param  pHeader: Header Buffer to be filled
  * @param  pWaveFormatStruct: Pointer to the wave structure to be filled.
  * @retval 0 if passed, !0 if failed.
  */
static uint32_t WavProcess_HeaderUpdate(uint8_t* pHeader, WAVE_FormatTypeDef* pWaveFormatStruct)
{
  /* Write the file length ---------------------------------------------------*/
  /* The sampling time: this value will be written back at the end of the
     recording operation.  Example: 661500 Btyes = 0x000A17FC, byte[7]=0x00, byte[4]=0xFC */
  pHeader[4] = (uint8_t)(BufferCtl.fptr);
  pHeader[5] = (uint8_t)(BufferCtl.fptr >> 8);
  pHeader[6] = (uint8_t)(BufferCtl.fptr >> 16);
  pHeader[7] = (uint8_t)(BufferCtl.fptr >> 24);
  /* Write the number of sample data -----------------------------------------*/
  /* This variable will be written back at the end of the recording operation */
  BufferCtl.fptr -=44;
  pHeader[40] = (uint8_t)(BufferCtl.fptr);
  pHeader[41] = (uint8_t)(BufferCtl.fptr >> 8);
  pHeader[42] = (uint8_t)(BufferCtl.fptr >> 16);
  pHeader[43] = (uint8_t)(BufferCtl.fptr >> 24);

  /* Return 0 if all operations are OK */
  return 0;
}

AUDIO_ErrorTypeDef recordStart()
{
	FRESULT res; /* FatFs function common result code */
	uint32_t byteswritten = 0;
	uwVolume = 100;

	/* Create a new file system */
	res = f_open(&SDFile, REC_WAVE_NAME, FA_CREATE_ALWAYS | FA_WRITE);
	if( res != FR_OK)
	{
		serialPrintln(&vcp,"cannot open file, code error : %d",res);
	}
	else
	{
		serialPrintln(&vcp,"open file");
		/* Initialize header file */
		WavProcess_EncInit(DEFAULT_AUDIO_IN_FREQ, pHeaderBuff);

		/* Write header file */
		if(f_write(&SDFile, pHeaderBuff, 44, (void*)&byteswritten) == FR_OK)
		{
			if(byteswritten != 0)
			{
			  serialPrintln(&vcp,"start record");

			  BSP_AUDIO_IN_Init(DEFAULT_AUDIO_IN_FREQ, DEFAULT_AUDIO_IN_BIT_RESOLUTION, 1);
			  BSP_AUDIO_IN_Record((uint16_t*)&BufferCtl.pcm_buff[0], AUDIO_IN_PCM_BUFFER_SIZE);
			  BufferCtl.fptr = byteswritten;
			  BufferCtl.pcm_ptr = 0;
			  BufferCtl.offset = 0;
			  BufferCtl.wr_state = BUFFER_EMPTY;
			  HAL_GPIO_WritePin(GPIOI, GPIO_PIN_1, SET);
			}
		}
	}

	return (AUDIO_ERROR_NONE);
}

AUDIO_ErrorTypeDef recordProcess()
{
	uint32_t elapsed_time;
	static uint32_t prev_elapsed_time = 0xFFFFFFFF;

	FRESULT res; /* FatFs function common result code */
	uint32_t byteswritten = 0;


	/* MAX Recording time reached, so stop audio interface and close file */
	if(BufferCtl.fptr >= REC_SAMPLE_LENGTH)
	{
	  return (AUDIO_ERROR_EOF);
	}

	if (BufferCtl.wr_state == BUFFER_FULL)
	{
		for(int i = 0; i < AUDIO_IN_PCM_BUFFER_SIZE/2; i ++)
		{
			buffer[i] =  BufferCtl.pcm_buff[BufferCtl.offset + i*4];
		}

		/* write buffer in file */
		res = f_write(&SDFile, (uint16_t*)(buffer),AUDIO_IN_PCM_BUFFER_SIZE/2,(void*)&byteswritten);
		if(res != FR_OK)
		{
			serialPrintln(&vcp, "cannot store data, code error : %d",res);
		}
		BufferCtl.fptr += byteswritten;
		BufferCtl.wr_state = BUFFER_EMPTY;
	}

	/* Display elapsed time */
	elapsed_time = BufferCtl.fptr / (DEFAULT_AUDIO_IN_FREQ * DEFAULT_AUDIO_IN_CHANNEL_NBR * 2);
	if(prev_elapsed_time != elapsed_time)
	{
	  prev_elapsed_time = elapsed_time;
	}

	return (AUDIO_ERROR_NONE);
}

AUDIO_ErrorTypeDef recordStop()
{
	FRESULT res; /* FatFs function common result code */
	uint32_t byteswritten = 0;

    /* Stop recorder */
    BSP_AUDIO_IN_Stop(CODEC_PDWN_SW);
    HAL_Delay(150);

    /* Move file pointer of the file object */
    res = f_lseek(&SDFile, 0);
    if(res != FR_OK)
    {
    	serialPrintln(&vcp, "f_lseek error, code error : %d",res);
    }
    else
    {
		/* Update the wav file header save it into wav file */
		WavProcess_HeaderUpdate(pHeaderBuff, &WaveFormat);

		res = f_write(&SDFile, pHeaderBuff, sizeof(WAVE_FormatTypeDef), (void*)&byteswritten);
		if(res != FR_OK)
		{
			serialPrintln(&vcp, "cannot end file, code error : %d",res);
		}
		else
		{
			serialPrintln(&vcp, "end of file",res);
		}
    }
    /* Close file */
    f_close(&SDFile);

    HAL_GPIO_WritePin(GPIOI, GPIO_PIN_1, RESET);

    serialPrintln(&vcp, "recording success");

    return (AUDIO_ERROR_NONE);
}

void BSP_AUDIO_IN_HalfTransfer_CallBack(void)
{
	BufferCtl.wr_state = BUFFER_FULL;
	BufferCtl.offset = 0;

}

void BSP_AUDIO_IN_TransferComplete_CallBack(void)
{
	BufferCtl.wr_state = BUFFER_FULL;
	BufferCtl.offset = AUDIO_IN_PCM_BUFFER_SIZE;
}

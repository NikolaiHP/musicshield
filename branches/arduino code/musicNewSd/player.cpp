#include <WProgram.h>
#include "string.h"
#include "player.h"
#include "config.h"
#include "vs10xx.h"
//#include "NewSoftSerial.h"
#include <SdFat.h>
#include <SdFatUtil.h>
#include "newSDLib.h"
//extern NewSoftSerial mySerial;

int playStop = 0;
unsigned char g_volume = 40;//used for controling the volume
int redPwm = 200;//used for controling the brightness of red led
unsigned int greenFreq = 5000;//used for controling the flash frequency of green led


void ControlLed()
{
	static unsigned char greenOnOff = 1;
	
	
	if( 0 == greenFreq--)
	{
		greenOnOff = 1-greenOnOff;
		greenFreq = 5000;
	}
	if(greenOnOff)
	{
		GREEN_LED_ON();
	}
	else
	{
		GREEN_LED_OFF();
	}	

	analogWrite(9,redPwm);
	
}

/*
void CheckKey()
{
  //static unsigned char volume = 40;
  static unsigned int vu_cnt = 1000;//volume up interval
  static unsigned int vd_cnt = 1000;//volume down interval

  
  if(0 == PSKey)
  {
  	playStop = 1-playStop;
	delay(20);
	while(0 == PSKey);
	delay(20);

  }
  

  if(0 == NTKey)
  {
  	playingState = PS_NEXT_SONG;
	delay(20);
	while(0 == NTKey);
	delay(20);
  }
  else if(0 == BKKey)
  {
    playingState = PS_PREVIOUS_SONG;
	delay(20);
	while(0 == BKKey);
	delay(20);
  }
  else if(0 == VUKey)
  {
  	if(--vu_cnt == 0)
	{
    		if (g_volume-- == 0) g_volume = 0; //Change + limit to 0 (maximum volume)
    
		Mp3SetVolume(g_volume,g_volume);		

		redPwm = (175-g_volume)*3>>1;
		if(redPwm >255)
		{
			redPwm = 255;
		}
		if(redPwm < 0)
		{
			redPwm = 0;
		}
		
		//Serial.println(redPwm,DEC);
		vu_cnt = 1000;
	 }
  }
  else if (0 == VDKey)
  {
    if(--vd_cnt == 0)
	{
      		if (g_volume++ == 254) g_volume = 254; //Change + limit to 254 (minimum vol)
	
	 	Mp3SetVolume(g_volume,g_volume);

		redPwm = 305-(g_volume<<1);
		if(redPwm >255)
		{
			redPwm = 255;
		}
		if(redPwm < 0)
		{
			redPwm = 0;
		}
		//Serial.println(redPwm,DEC);
	  	vd_cnt = 1000;
	 }
       	         
  }
  
  
}

void IPODCommandProcess()
{
	if(mySerial.available())
	{
		delay(10);

		int cmdBuffer[40];
		int len = mySerial.available();
		int i = 0;
		Serial.print(len,HEX);
		while(len--)
		{
			cmdBuffer[i++] = mySerial.read();
			//Serial.print(cmdBuffer[i-1],HEX);
		}
		
		if(0xff == cmdBuffer[0] && 0x55 == cmdBuffer[1] && 0x03 == cmdBuffer[2]  && 0x02 == cmdBuffer[3])
		{
			switch (cmdBuffer[5])
			{
				case 0x01:playStop = 1-playStop;break;
				case 0x02:if (g_volume-- == 0) g_volume = 0;Mp3SetVolume(g_volume,g_volume);break;
				case 0x04:if (g_volume++ == 254) g_volume = 254;Mp3SetVolume(g_volume,g_volume);break;
				case 0x08:playingState = PS_NEXT_SONG;break;
				case 0x10:playingState = PS_PREVIOUS_SONG;break;				
				case 0x20:playingState = PS_NEXT_SONG;break;//it should be next Album
				case 0x40:playingState = PS_PREVIOUS_SONG;break;//it shuld be previous Album
				case 0x80:playStop = 1-playStop;break;
				default: break;
			}

			//wait remote button release
			unsigned char continueZeros = 0;
			while(1)
			{
				if(mySerial.available())
				{
					if(mySerial.read() == 0)
					{
						continueZeros++;
						//Serial.print(continueZeros,DEC);
						if(5 == continueZeros)
							break;
					}
					else
					{
						continueZeros = 0;
					}
				}
			}
			
		}
	}
}
*/

/** This function is called when the player is playing a song
 and there is free processor time. The basic task of this
 function is to implement the player user interface.*/
/*void AvailableProcessorTime()
{
	
	do
	{
  		CheckKey();
  
 		 //IPODCommandProcess();
		 
		if(0 == playStop)
		{
			GREEN_LED_ON();
		}	
	}while(0 == playStop);
	
  	//do other things
	ControlLed();
	
}*/

int playFile(char *fileName)
{

  Mp3SoftReset();
  
  openFile(fileName);//open music file

  int readLen = 0;
  byte readBuf[READ_BUF_LEN];
  byte *tp = readBuf;
  while(1)
  {
    readLen = readFile(readBuf,READ_BUF_LEN);//read file content length of 512 every time
    tp = readBuf;
    //Serial.println(readLen);

    Mp3SelectData();

    while (tp < readBuf+readLen)
    {
      if (!MP3_DREQ)
      {
        while (!MP3_DREQ)
        {
          Mp3DeselectData();
          //AvailableProcessorTime();//do interactive things
          Mp3SelectData();
        }
      }
      // Send music content data to VS10xx 
      SPIPutChar(*tp++);
    }
    
    SPIWait();
    Mp3DeselectData();
    
    if(readLen < READ_BUF_LEN)
    {
      Mp3WriteRegister(SPI_MODE,0,SM_OUTOFWAV);
      SendZerosToVS10xx();
      break;
    }
  };
  Serial.println("played over\r\n");
  
  if(file.close() == 0)//close file
  {
    error("close file failed");
  }
  return 0; //OK Exit
}


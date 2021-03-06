/*
# This file is part of the PiXtend(R) Project.
#
# For more information about PiXtend(R) and this program,
# see <http://www.pixtend.de> or <http://www.pixtend.com>
#
# Copyright (C) 2015 Nils Mensing, Christian Strobel
# Qube Solutions UG (haftungsbeschränkt), Luitgardweg 18
# 71083 Herrenberg, Germany 
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <pixtend.h>

#define VERSION "0.5.0"

static int fd;

void signal_callback_handler(int signum)
{
	printf("\n\nCaught signal %d\n",signum);
	serialFlush(fd);
	printf("Flushed serial device\n");
	serialClose(fd);
	printf("Closed serial device\n");
	exit(signum);
}

void help()
{
	printf("PiXtend Tool - www.pixtend.de - Version %s\n", VERSION);
	printf("usage: sudo pixtendtool [OPTION] [VALUE(s)]\n");
	printf("\n");
	printf("available options:\n");
	printf("\t-h \t\t\t\tprint this help\n");
	printf("\t-do VALUE \t\t\tset the digital output byte to VALUE[0-255]\n");
	printf("\t-di \t\t\t\tget the digital input byte\n");
	printf("\t-di BIT\t\t\t\tget the digital input BIT[0-7]\n");	
	printf("\t-ai CHANNEL \t\t\tget the analog input CHANNEL[0-3] raw value\n");
	printf("\t-ai CHANNEL REF \t\tget the analog input CHANNEL[0-3] value based on REF[5V/10V]\n");
	printf("\t-aic VALUE0 VALUE1 \t\twrite AI Control Registers to VALUEx[0-255]\n");
	printf("\t-ao CHANNEL VALUE \t\tset the analog output CHANNEL[0-1] to VALUE[0-1023]\n");	
	printf("\t-rel VALUE \t\t\tset the relay output byte to VALUE[0-255]\n");
	printf("\t-gw VALUE \t\t\twrite GPIO output byte to VALUE[0-255]\n");
	printf("\t-gr \t\t\t\tread GPIO input byte\n");
	printf("\t-gc VALUE \t\t\twrite GPIO control register to VALUE[0-255]\n");	
	printf("\t-tr CHANNEL [TYPE]\t\tread temperature from CHANNEL[0-3] of TYPE[dht11/dht22]\n");
	printf("\t-hr CHANNEL [TYPE]\t\tread humidity from CHANNEL[0-3] of TYPE[dht11/dht22]\n");	
	printf("\t-srv CHANNEL VALUE \t\tset a servo VALUE[0-255] for CHANNEL[0-1]\n");
	printf("\t-pwm CHANNEL VALUE \t\tset a pwm VALUE[0-65535] for CHANNEL[0-1]\n");
	printf("\t-pwmc VALUE0 VALUE1 VALUE2 \twrite PWM control registers to VALUEx[0-255] \n");
	printf("\t-shw MODE\t\t\tchange serial hardware MODE[rs232/rs485]\n");
	printf("\t-swc CHAR\t\t\twrite a CHAR on RS232/485\n");
	printf("\t-sws STRING\t\t\twrite a STRING on RS232/485 (max. len=16)\n");
	printf("\t-sr \t\t\t\tread data from RS232/485 until Ctrl^C\n");
	printf("\t-ucc VALUE \t\t\twrite the microcontroller control register to VALUE[0-255]\n");
	printf("\t-ucr \t\t\t\treset the microcontroller\n");
	printf("\t-ucs \t\t\t\tread microcontroller status register\n");
	printf("\t-ucv \t\t\t\tread microcontroller version\n");
	printf("\t-rasp VALUE \t\t\twrite the RaspberryPi status register to VALUE[0-255]\n");
	printf("\n");
	printf("Application Notes about the correct usage of Control and Status Bytes\n");
	printf("can be found at http://www.pixtend.de/pixtend/downloads/\n");
	
	return;
}

int limitPar(int *value, int min, int max, const char* par) 
{	
	int ret = 0;
	if(*value < min) {
		*value = min;
		ret = 1;
	}		
	if(*value > max) {
		*value = max;
		ret = 2;
	}
	if(ret) 
		printf("Warning: %s range exceeded. Allowed range: [%d - %d]\n",par,min,max);
	
	return ret;
}

int main(int argc, char *argv[]) 
{
	int i=0;
	int nRet;

	uint16_t uiRet;
	errno = 0;
	
	if(argc > 1)
	{
		if(strcmp(argv[1],"-h")==0)
		{
			help();
		}
		else if(strcmp(argv[1],"-do")==0 && argc==3)
		{
			int value;
			value = atoi(argv[2]);
			limitPar(&value, 0,255, "VALUE");

			Spi_Setup(0);
			Spi_Set_Dout(value);
			printf("Digital output byte set to [%d]\n",value);
		}
		else if(strcmp(argv[1],"-di")==0 && argc==2)
		{
			Spi_Setup(0);
			nRet = Spi_Get_Din();
			printf("Digital input byte is [%d]\n",nRet);
		}
		else if(strcmp(argv[1],"-di")==0 && argc==3)
		{
			int bit;
			bit = atoi(argv[2]);
			limitPar(&bit, 0,7, "BIT");
			
			Spi_Setup(0);
			uint8_t fvalue;
			if(Spi_Get_Din() & (1<<bit)) {
				fvalue = 1;
			}
			else {
				fvalue = 0;
			}			
			printf("Digital input bit %d value is [%d]\n",bit,fvalue);
		}
		else if(strcmp(argv[1],"-ai")==0 && argc==3)
		{
			int channel = atoi(argv[2]);
			limitPar(&channel,0,3,"CHANNEL");		
			
			Spi_Setup(0);
			uiRet = Spi_Get_Ain(channel);
			printf("Analog input %d value is [%d]\n",channel,uiRet);
		}
		else if(strcmp(argv[1],"-ai")==0 && argc==4)
		{
			int channel = atoi(argv[2]);
			limitPar(&channel,0,3,"CHANNEL");		
			
			Spi_Setup(0);
			uiRet = Spi_Get_Ain(channel);
			
			if(channel >= 0 && channel <= 1) 
			{
				if (strcmp(argv[3],"5V")==0) {
					printf("Analog input %d voltage is [%2.2f]V\n",channel,(float)uiRet*5.0/1024.0);
				}
				else if(strcmp(argv[3],"10V")==0) {
					printf("Analog input %d voltage is [%2.2f]V\n",channel,(float)uiRet*10.0/1024.0);
				}
				else  {
					printf("Error: Unknown reference voltage %s. Valid Options are: [5V/10V]\n",argv[3]);
				}			
			}
			else if (channel >=2 && channel <=3) {
				printf("Analog input %d current is [%2.2f]mA\n",channel,(float)uiRet*0.024194115990990990990990990991);
			}
						
			
		}
		else if(strcmp(argv[1],"-aic")==0 && argc==4)
		{
			int value0 = atoi(argv[2]);
			limitPar(&value0,0,255,"VALUE0");
			
			int value1 = atoi(argv[3]);
			limitPar(&value1,0,255,"VALUE1");
			
			Spi_Setup(0);
			Spi_Set_AiControl(value0,value1);
			printf("AI Control Registers set to [%d], [%d]\n",value0,value1);
		}
		else if(strcmp(argv[1],"-ao")==0 && argc==4)
		{
			int channel = atoi(argv[2]);
			limitPar(&channel,0,1,"CHANNEL");			
			
			int value = atoi(argv[3]);
			limitPar(&value,0,1023,"VALUE");
			
			Spi_Setup(1);
			Spi_Set_Aout(channel,(uint16_t)value);
			printf("Analog output channel %d set to [%d]\n",channel, value);
		}
		else if(strcmp(argv[1],"-rel")==0 && argc==3)
		{
			int value;
			value = atoi(argv[2]);
			limitPar(&value, 0,255, "VALUE");
			
			uint8_t fvalue = (uint8_t) value;
			
			Spi_Setup(0);
			Spi_Set_Relays(fvalue);
			printf("Relay outputs set to [%d]\n",fvalue);
		}
		
		
		else if(strcmp(argv[1],"-gr")==0 && argc==2)
		{			
			Spi_Setup(0);
			nRet = Spi_Get_Gpio();
			printf("GPIO byte value is [%d]\n",nRet);
		}
		else if(strcmp(argv[1],"-gw")==0 && argc==3)
		{
			int value = atoi(argv[2]);
			limitPar(&value, 0,255, "VALUE");
			
			Spi_Setup(0);
			Spi_Set_Gpio(value);
			printf("GPIO byte set to [%d]\n",value);
		}
		else if(strcmp(argv[1],"-gc")==0 && argc==3)
		{
			int value = atoi(argv[2]);
			limitPar(&value, 0,255, "VALUE");
			
			Spi_Setup(0);
			Spi_Set_GpioControl(value);
			printf("GPIO control register set to [%d]\n",value);
		}
		else if(strcmp(argv[1],"-tr")==0 && (argc==3||argc==4)) {
			
			int channel = atoi(argv[2]);
			limitPar(&channel,0,3,"CHANNEL");
			
			Spi_Setup(0);
			uiRet = Spi_Get_Temp(channel);
			
			if ((argc==4 && strcmp(argv[3],"dht22")==0) || argc==3) {			
				printf("Temperature Channel %d: %2.1f deg[C]\n",channel,(float)uiRet/10.0);
			}
			else if(argc==4 && strcmp(argv[3],"dht11")==0) {
				printf("Temperature Channel %d: %2.1f deg[C]\n",channel,(float)(uiRet>>8)*1.0);
			}
			else  {
				printf("Error: Unknown Sensor Type %s. Valid Options are: [dht11/dht22]\n",argv[3]);
			}						
		}
		else if(strcmp(argv[1],"-hr")==0 && (argc==3 || argc==4))
		{
			int channel = atoi(argv[2]);
			limitPar(&channel,0,3,"CHANNEL");
			
			Spi_Setup(0);
			uiRet = Spi_Get_Hum(channel);
			
			if ((argc==4 && strcmp(argv[3],"dht22")==0) || argc==3) {			
				printf("Humidity Channel %d: %2.1f %%\n",channel,(float)uiRet/10.0);
			}
			else if(argc==4 && strcmp(argv[3],"dht11")==0) {
				printf("Humidity Channel %d: %2.1f %%\n",channel,(float)(uiRet>>8)*1.0);
			}
			else  {
				printf("Error: Unknown sensor type %s. Valid options are: [dht11/dht22]\n",argv[3]);
			}						
		}
		else if(strcmp(argv[1],"-srv")==0 && argc==4)
		{	
			int channel = atoi(argv[2]);
			limitPar(&channel,0,1,"CHANNEL");
			
			int value = atoi(argv[3]);
			limitPar(&value, 0,255, "VALUE");
			
			Spi_Setup(0);
			nRet = Spi_Set_Servo(channel,value);
			printf("Servo Channel %d set to [%d]\n",channel,value);
		}
		else if(strcmp(argv[1],"-pwm")==0 && argc==4)
		{	
			int channel = atoi(argv[2]);
			limitPar(&channel,0,1,"CHANNEL");
			
			int value = atoi(argv[3]);
			limitPar(&value, 0,65535, "VALUE");
		
			Spi_Setup(0);
			nRet = Spi_Set_Pwm(channel,value);
			printf("PWM Channel %d set to [%d]\n",channel,value);
		}
		else if(strcmp(argv[1],"-pwmc")==0 && argc==5)
		{
			int value0 = atoi(argv[2]);
			limitPar(&value0, 0,255, "VALUE0");
			int value1 = atoi(argv[3]);
			limitPar(&value1, 0,255, "VALUE1");
			int value2 = atoi(argv[4]);
			limitPar(&value2, 0,255, "VALUE2");
			
			Spi_Setup(0);
			Spi_Set_PwmControl(value0,value1,value2);
			printf("PWM control registers set to [%d], [%d], [%d]\n",value0,value1,value2);
		}
		else if(strcmp(argv[1],"-shw")==0 && argc==3)
		{
			if (strcmp(argv[2],"rs232")==0) {
				Change_Serial_Mode(0);
				printf("Changed serial hardware mode to RS232\n");
			}
			else if(strcmp(argv[2],"rs485")==0) {
				printf("Changed serial hardware mode to RS485\n");
			}
			else  {
				printf("Error: Unknown hardware mode %s. Valid options are: [rs232/rs485]\n",argv[2]);
			}		
		}
		else if(strcmp(argv[1],"-swc")==0 && argc==3)
		{
			fd = serialOpen("/dev/ttyAMA0",9600);
			if(fd==-1)
			{
				printf("Error: Couldn't open serial device\n");
			}
			else
			{
				printf("Opened serial device [fd=%d]\n",fd);
				serialPutchar(fd,*argv[2]);
				printf("put char %c\n",*argv[2]);
				serialFlush(fd);
				printf("Flushed serial device\n");
				serialClose(fd);
				printf("Closed serial device\n");
			}
		}
		else if(strcmp(argv[1],"-sws")==0 && argc>=3)
		{
			char str[256];
			int i;
			
			strcpy(str,argv[2]);
			for(i=3;i<argc;i++)
			{
				strcat(str," ");
				strcat(str,argv[i]);
			}
			
			fd = serialOpen("/dev/ttyAMA0",9600);
			if(fd==-1)
			{
				printf("Error: Couldn't open serial device\n");
			}
			else
			{
				printf("Opened serial device [fd=%d]\n",fd);
				serialPuts(fd,str);
				printf("put string %s\n",str);
				serialFlush(fd);
				printf("Flushed serial device\n");
				serialClose(fd);
				printf("Closed serial device\n");
			}
		}
		else if(strcmp(argv[1],"-sr")==0 && argc==2)
		{
			int chr;
			int nRet;
			
			signal(SIGINT, signal_callback_handler);
			
			fd = serialOpen("/dev/ttyAMA0",9600);
			if(fd==-1)
			{
				printf("Error: Couldn't open serial device\n");
			}
			else
			{
				printf("Opened serial device [fd=%d]\n",fd);
				printf("Recv: ");
				fflush(stdout);
				while(1)
				{
					nRet = serialDataAvail(fd);
					if (nRet>=0)
					{
						chr = serialGetchar(fd);
						if(chr!=-1)
						{
							printf("%c",chr);
							fflush(stdout);
						}
					}
					else
					{
						printf("Error: serialDataAvail=-1\n");
						serialFlush(fd);
						printf("Flushed serial device\n");
						serialClose(fd);
						printf("Closed serial device\n");
						return 0;
					}
					
				}
			}
		}
		else if(strcmp(argv[1],"-ucc")==0 && argc==3)
		{
			int value = atoi(argv[2]);
			limitPar(&value, 0,255, "VALUE");
			
			Spi_Setup(0);
			Spi_Set_UcControl(value);
			printf("Microcontroller control register set to [%d]\n",value);
		}		
		else if(strcmp(argv[1],"-ucr")==0 && argc==2)
		{
			nRet = Spi_uC_Reset();
			printf("Microcontroller reset successful!\n");
		}
		else if(strcmp(argv[1],"-ucs")==0 && argc==2)
		{
			Spi_Setup(0);
			nRet = Spi_Get_uC_Status();
			printf("Microcontroller status register is [%d]\n", nRet);
		}
		else if(strcmp(argv[1],"-ucv")==0 && argc==2)
		{
			uint16_t version;
			Spi_Setup(0);
			version = Spi_Get_uC_Version();
			printf("Microcontroller version is %d.%d\n", (uint8_t)(version>>8), (uint8_t)(version & 0xFF));
		}
		else if(strcmp(argv[1],"-rasp")==0 && argc==3)
		{
			int value = atoi(argv[2]);
			limitPar(&value, 0,255, "VALUE");
			
			Spi_Setup(0);
			Spi_Set_RaspStat(value);
			printf("RaspberryPi status register set to [%d]\n",value);
		}
		else
		{
			printf("Wrong command: ");
			for(i=0;i<argc;i++)
			{
				printf("%s ",argv[i]);
			}
			printf("\n\n");
			help();
		}
	}
	else
	{
		help();
	}
	
	return 0;
}

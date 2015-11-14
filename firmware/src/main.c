/*
 	bldc-drive Cheap and simple brushless DC motor driver designed for CNC applications using STM32 microcontroller.
	Copyright (C) 2015 Pekka Roivainen

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/


#include <stdint.h>
#include <stdio.h>
#include <stddef.h>

#include <stm32f10x_gpio.h>
#include <stm32f10x_tim.h>
#include <stm32f10x_rcc.h>



#include "pwm.h"
#include "hall.h"
#include "adc.h"
#include "input.h"
#include "encoder.h"
#include "pid.h"
#include "usart.h"
#include "configuration.h"
#include "input.h"

volatile uint8_t dir;
volatile servoConfig s;



int
main()
{


	motor_running=0;
	updateCtr=0;
	dir=1;

	FLASH_Unlock();
	getConfig();
	FLASH_Lock();
	initUSART(s.usart_baud);
	printConfiguration();

	initPWM();
	initADC();
	if( s.commutationMethod == commutationMethod_HALL)
	{
		initHALL();
	}

	if(s.inputMethod == inputMethod_stepDir)
	{
		initStepDirInput();
	}
	else if (s.inputMethod == inputMethod_pwmVelocity)
	{
		initPWMInput();
	}
	if(s.inputMethod == inputMethod_stepDir || s.commutationMethod == commutationMethod_Encoder)
	{
		initEncoder();
	}

	if(s.inputMethod == inputMethod_stepDir)
	{
		initPid();
	}
	initLeds();
	errorInCommutation=1;
	uint8_t ena;
	//check if ENA is on already at start. If it is, start motor.
#if ENA_POLARITY == 1
		ena = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_5);
#else
		ena = (~(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_5)))&1;
#endif
	if(ena)
	{
		pwm_motorStart();
		ENABLE_LED_ON;
	}

  while (1)
    {
	  getEncoderCount();
	  if(s.commutationMethod == commutationMethod_Encoder)
	  {
		  if(encoder_commutation_pos != encoder_commutation_table[encoder_shaft_pos])
		  {
			  //usart_sendStr("commutation to ");
			  //usart_sendChar(encoder_commutation_table[encoder_shaft_pos]+48);
			  encoder_commutation_pos = encoder_commutation_table[encoder_shaft_pos];
			  pwm_Commute(encoder_commutation_pos);
			//  usart_sendStr("\n\r");
		  }
	  }
	  if(s.inputMethod==inputMethod_stepDir)
		  updatePid();

    }
}


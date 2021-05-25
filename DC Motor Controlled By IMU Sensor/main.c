/*
 * ATmega16 Interface with MPU-6050
 * http://www.electronicwings.com
 *
 */ 

/*
	DA6
	
	1. 	Interface the provided  MPU-6050 6-DOF IMU Sensor to the ATmega328p using the I2C interface.
	Using the earlier developed code for UART, display the accelerometer and gyro data to the UART Terminal.

	2.	Develop an algorithm to smooth the data from MPU-6050 6-DOF IMU Sensor. Display the smooth
	accelerometer gyro data to the UART Terminal.

	3.	Determine the roll angle of the MPU-6050. Using MPU6050 as the input, control the direction and
	speed of the DC motor. If the roll is negative the motor rotates anti-clockwise and if rolls positive the motor
	rotates clockwise. The speed of the motor is faster based on the angle of roll magnitude.
*/

#define F_CPU 16000000UL									/* Define CPU clock Frequency e.g. here its 16MHz */
#include <avr/io.h>										/* Include AVR std. library file */
#include <util/delay.h>									/* Include delay header file */
#include <inttypes.h>									/* Include integer type header file */
#include <stdlib.h>										/* Include standard library file */
#include <stdio.h>										/* Include standard library file */
#include <avr/interrupt.h>
#include "MPU6050_def.h"							/* Include MPU6050 register define file */
#include "i2c_master.h"							/* Include I2C Master header file */
#include "uart.h"							/* Include USART header file */

float Acc_x,Acc_y,Acc_z,Temperature,Gyro_x,Gyro_y,Gyro_z;

void MPU6050_Init()										/* Gyro initialization function */
{
	_delay_ms(150);										/* Power up time >100ms */
	I2C_Start_Wait(0xD0);								/* Start with device write address */
	I2C_Write(SMPLRT_DIV);								/* Write to sample rate register */
	I2C_Write(0x07);									/* 1KHz sample rate */
	I2C_Stop();

	I2C_Start_Wait(0xD0);
	I2C_Write(PWR_MGMT_1);								/* Write to power management register */
	I2C_Write(0x01);									/* X axis gyroscope reference frequency */
	I2C_Stop();

	I2C_Start_Wait(0xD0);
	I2C_Write(CONFIG);									/* Write to Configuration register */
	I2C_Write(0x00);									/* Fs = 8KHz */
	I2C_Stop();

	I2C_Start_Wait(0xD0);
	I2C_Write(GYRO_CONFIG);								/* Write to Gyro configuration register */
	I2C_Write(0x18);									/* Full scale range +/- 2000 degree/C */
	I2C_Stop();

	I2C_Start_Wait(0xD0);
	I2C_Write(INT_ENABLE);								/* Write to interrupt enable register */
	I2C_Write(0x01);
	I2C_Stop();
}

void MPU_Start_Loc()
{
	I2C_Start_Wait(0xD0);								/* I2C start with device write address */
	I2C_Write(ACCEL_XOUT_H);							/* Write start location address from where to read */ 
	I2C_Repeated_Start(0xD1);							/* I2C start with device read address */
}

void Read_RawValue()
{
	MPU_Start_Loc();									/* Read Gyro values */
	Acc_x = (((int)I2C_Read_Ack()<<8) | (int)I2C_Read_Ack());
	Acc_y = (((int)I2C_Read_Ack()<<8) | (int)I2C_Read_Ack());
	Acc_z = (((int)I2C_Read_Ack()<<8) | (int)I2C_Read_Ack());
	Temperature = (((int)I2C_Read_Ack()<<8) | (int)I2C_Read_Ack());
	Gyro_x = (((int)I2C_Read_Ack()<<8) | (int)I2C_Read_Ack());
	Gyro_y = (((int)I2C_Read_Ack()<<8) | (int)I2C_Read_Ack());
	Gyro_z = (((int)I2C_Read_Ack()<<8) | (int)I2C_Read_Nack());
	I2C_Stop();
}

ISR(INT0_vect)
{
	_delay_ms(50);							/* Software de-bouncing control delay */
}

int main()
{
	char buffer[20], float_[10];
	float Xa,Ya,Za,t;
	float Xg=0,Yg=0,Zg=0;
	
	/////////////////////Initialize Variables///////////////////////////////
	float smoothroll = 0;		//Value to add roll value to itself for AVG
	int roll = 0;				//Roll Value
	int scalefactor = 0;		//Scale Factor to use 80% Max DC power
	int temp = 0;				//Temp value to get absolute value for SPEED
	volatile int forward = 0;	//Flag to check if motor is going forward
	volatile int reverse = 0;	//Flag to check if motor is going reverse
	///////////////////////////////////////////////////////////////////////
	
	I2C_Init();											/* Initialize I2C */
	MPU6050_Init();										/* Initialize MPU6050 */
	USART_Init(9600);									/* Initialize USART with 9600 baud rate */

	/////////////////////DC Motor Code/////////////////////////////////////

	DDRD &= ~(1<<DDD2);							/* Make INT0 pin as Input */
	PORTD |= (1 << DDD2);						// turn On the Pull-up
	DDRD |= (1<<DDD6) | (1<<DDD4) | (1<<DDD5);						/* Make OC0 pin as Output */
	//PORTD |= (1<<DDD4);
	//Begin Initialize Timer
	EICRA |= (1 << ISC01);						// set INT0 to trigger to falling edge
	EIMSK |= (1 << INT0);						// Turns on INT0
	sei();										/* Enable Global Interrupt */
	TCNT0 = 0;									/* Set timer0 count zero */
	TCCR0A |= (1<<WGM00)|(1<<WGM01)|(1<<COM0A1); //Timer 0
	TCCR0B |= (1<<CS00)|(1<<CS02);				/* Set Fast PWM with Fosc/64 Timer0 clock */

	//////////////////////End of Motor Initialization//////////////////////
	
	while(1)
	{
		/////////////////////TASK 2 Begin Smoothing Algorithm//////////////////////////////
		
		//Initialize Smooth Values
		smoothroll = 0;
		
		//Sample each Roll Value 100 times By moving their sums into a temp float variable
		for (int i = 0; i < 100; i++)
		{
			Read_RawValue();
			smoothroll += (atan2(-Acc_y, Acc_z)*180.0)/M_PI;
		}
		
		//Calculate Roll and Take average of the 100 sample data sums
		roll = (smoothroll/100); //MAX ANGLE IS 180 DEGREES!
		
		///////////////////////////////////////////////////////////////////////////////////
		
		
		
		
		
		
		//Calculate Sensor Data
		
		Xa = Acc_x/16384.0;								/* Divide raw value by sensitivity scale factor to get real values */
		Ya = Acc_y/16384.0;
		Za = Acc_z/16384.0;
		
		Xg = Gyro_x/16.4;
		Yg = Gyro_y/16.4;
		Zg = Gyro_z/16.4;

		t = (Temperature/340.00)+36.53;					/* Convert temperature in °/c using formula */

		scalefactor = (roll/0.88); //204 max speed, 80% DC Motor Power
		temp = abs(scalefactor);
		
		
		
		
		
		
		
		
		/////////////////////TASK 3 CONTROL MOTOR SPEED AND DIRECTION BY ROLL//////////////
		
		OCR0A = temp;	//SET SPEED TO ABSOLUTE VALUE OF SCALED ROLL VALUE

		if (roll > 0)	//even, go forward
		{
			forward = 1;
			if (reverse == 1) //IF TRUE, STOP MOTOR AND WAIT A BIT
			{	
				//Message to alert user it is about to go forward
				dtostrf( roll, 3, 2, float_ );					
				sprintf(buffer," PREPARING FOR FORWARD \t \n",float_);
				USART_SendString(buffer);
				
				//STOP MOTOR FOLLOWED BY DELAY, SET AIN1, AIN2 = 00
				PORTD &= ~(1<<DDD4);
				PORTD &= ~(1<<DDD5);
				_delay_ms(500); //WAIT A BIT
			}
			reverse = 0;
			
			//SET AIN1, AIN2 = 10
			PORTD |= (1<<DDD4);
			PORTD &= ~(1<<DDD5);
		}
		
		if (roll < 0) //negative, going reverse
		{
			if (forward == 1) //IF TRUE, STOP MOTOR AND WAIT A BIT
			{
				//Message to alert user it is about to go forward
				dtostrf( roll, 3, 2, float_ );
				sprintf(buffer," PREPARING FOR REVERSE \t \n",float_);
				USART_SendString(buffer);
				
				//STOP MOTOR FOLLOWED BY DELAY, SET AIN1, AIN2 = 00
				PORTD &= ~(1<<DDD4);
				PORTD &= ~(1<<DDD5);
				_delay_ms(500); //WAIT A BIT
			}
			reverse = 1;
			forward = 0;
			
			//SET AIN1, AIN2 = 01
			PORTD &= ~(1<<DDD4);
			PORTD |= (1<<DDD5);
		}

		///////////////////////////////////////////////////////////////////////////////////








		/////////////////////TASK 1 DISPLAY SENSOR DATA TO UART TERMINAL///////////////////



		dtostrf( roll, 3, 2, float_ );					///* Take values in buffer to send all parameters over USART 
		sprintf(buffer," ROLL = %s g\t \n",float_);
		USART_SendString(buffer);

///*
		dtostrf( Xa, 3, 2, float_ );					///* Take values in buffer to send all parameters over USART 
		sprintf(buffer," Ax = %s g\t",float_);
		USART_SendString(buffer);

		dtostrf( Ya, 3, 2, float_ );
		sprintf(buffer," Ay = %s g\t",float_);
		USART_SendString(buffer);
		
		dtostrf( Za, 3, 2, float_ );
		sprintf(buffer," Az = %s g\t",float_);
		USART_SendString(buffer);

		dtostrf( t, 3, 2, float_ );
		sprintf(buffer," T = %s%cC\t",float_,0xF8);           ///* 0xF8 Ascii value of degree '°' on serial 
		USART_SendString(buffer);

		dtostrf( Xg, 3, 2, float_ );
		sprintf(buffer," Gx = %s%c/s\t",float_,0xF8);
		USART_SendString(buffer);

		dtostrf( Yg, 3, 2, float_ );
		sprintf(buffer," Gy = %s%c/s\t",float_,0xF8);
		USART_SendString(buffer);
		
		dtostrf( Zg, 3, 2, float_ );
		sprintf(buffer," Gz = %s%c/s\r\n",float_,0xF8);
		USART_SendString(buffer);
		//_delay_ms(250);
	//*/
	}
}
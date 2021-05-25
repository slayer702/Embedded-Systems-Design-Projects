/*
 * 
 *
 * Created: 8/10/2020 9:54:41 PM
 * 
 */ 

/*
 * LCD EXAMPLE
 *
2.	Using SPI protocol display the temperature sensor value (int only) of LM34/35 on to the GLCD.
*/

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include "nokia5110.h"

char buffer[10];                //Output of the itoa function

void adc_init(void);
void read_adc(void);
volatile unsigned int adc_temp;

/* Connect BL or B-LED to 3.3V if the GLCD is hot */

int main(void)
{
	adc_init();									//Initialize ADC
	
	for (;;) {
		read_adc();								//READ ADC
		nokia_lcd_init();						//INITIALIZE LCD
		nokia_lcd_clear();						//CLEAR LCD
		itoa(adc_temp, buffer, 10);				//Convert ADC Value to Char
		_delay_ms(125);							// wait a bit
		
		nokia_lcd_set_cursor(5, 0);				//SET LCD CURSOR
		nokia_lcd_write_string("Temp (F):",1);	//Write to LCD
		nokia_lcd_set_cursor(65, 0);			//SET LCD CURSOR
		nokia_lcd_write_string(buffer, 1);		//Write ADC Value to LCD in Fahrenheit
		nokia_lcd_render();								//Write screen to display
		_delay_ms(500);							// wait 500ms Before updating 
	}
}

//INIT ADC
void adc_init(void)
{
	ADMUX = (0<<REFS1) |					// Reference Selection Bits
	(1<<REFS0) |							// AVcc - external cap at AREF
	(0 << ADLAR) |							// ADC Left Adjust Result
	(1 << MUX2) |							// Analog Channel Selection Bits
	(0 << MUX1) |							// ADC5 (PC5)********
	(1 << MUX0);							//
	ADCSRA = (1<<ADEN) |					// ADC Enable
	(0 << ADSC) |							// ADC Start Conversation
	(0 << ADATE) |							// ADC Auto Trigger Enable
	(0 << ADIF) |							// ADC Interrupt Flag
	(0 << ADIE) |							// ADC Interrupt Enable
	(0 << ADPS2) |							// ADC Prescaler Select Bits
	(0 << ADPS1) |
	(0 << ADPS0);
}

//Read ADC PINS
void read_adc(void)
{
	unsigned char i = 4;
	adc_temp = 0;
	while (i--)
	{
		ADCSRA |= (1<<ADSC);
		while (ADCSRA & (1<<ADSC));
		adc_temp+= ADC;
		_delay_ms(50);
	}
	adc_temp = (adc_temp/4);				// Average of four samples
	adc_temp = (adc_temp/1024.0) * 5000/10;  //convert to Fahrenheit
	adc_temp = (adc_temp * 9)/5 + 32; //celsius to fahrenheit
}

/*
 * power.h
 *
 * 
 * Author: Karim Bouanane
 */ 

#ifndef POWER_H_
#define POWER_H_


#define disable_adc()		(ADCSRA &= (uint8_t)~(1 << ADEN))
#define enable_adc()		(ADCSRA |= (uint8_t)(1 << ADEN))

#define power_adc_on()      (PRR &= (uint8_t)~(1 << PRADC))
#define power_adc_off()     (PRR |= (uint8_t)(1 << PRADC))

#define power_ac_on()       (ACSR &= (uint8_t)~(1 << ACD))
#define power_ac_off()      (ACSR |= (uint8_t)(1 << ACD))

#define power_spi_on()      (PRR &= (uint8_t)~(1 << PRSPI))
#define power_spi_off()     (PRR |= (uint8_t)(1 << PRSPI))

#define power_timer0_on()   (PRR &= (uint8_t)~(1 << PRTIM0))
#define power_timer0_off()  (PRR |= (uint8_t)(1 << PRTIM0))

#define power_timer1_on()   (PRR &= (uint8_t)~(1 << PRTIM1))
#define power_timer1_off()  (PRR |= (uint8_t)(1 << PRTIM1))

#define power_timer2_on()   (PRR &= (uint8_t)~(1 << PRTIM2))
#define power_timer2_off()  (PRR |= (uint8_t)(1 << PRTIM2))

#define power_twi_on()      (PRR &= (uint8_t)~(1 << PRTWI))
#define power_twi_off()     (PRR |= (uint8_t)(1 << PRTWI))

#define power_usart0_on()   (PRR &= (uint8_t)~(1 << PRUSART0))
#define power_usart0_off()  (PRR |= (uint8_t)(1 << PRUSART0))

#define enable_pullup()		(MCUCR &= ~(1<<PUD))

/* 
 Important Note about turning off a peripheral
	
		Data sheet mention that peripheral when turned off, it freezes in its 
	current state with I/O registers becoming inaccessible. Also the resources
	used by this peripheral would remain unoccupied. So its suggested to disable
	the peripheral before stopping the clock to it.
	Example:
			disable_adc();
			power_adc_off();
 
 */


/*
 Important Note about Unconnected Pins
 
		Floating inputs should be avoided to reduce current consumption in all
	other modes where the digital inputs are enabled (Reset, Active Mode and
	Idle Mode). We need to use pull_up or pull_down to reduce power.
 
	Active Mode - 5V @16Mhz
 
 	  Pin Mode			 Current 
						Consumption
	Output High				21mA	
	Output Low				16mA
	
	Input H-Z(Floating)		24mA
	Input pull-up			16mA
	 
*/


#define lowPowerIO() \
do{					\
	DDRB = 0xFF;	\
	DDRC = 0xFF;	\
	DDRD = 0xFF;	\
	PORTB = 0;		\
	PORTC = 0;		\
	PORTD = 0;		\
}while(0)				


/*
 Lowering Frequency

	Measure of current from Lowering Frequency

	Active mode - 5V:

	Frequency	 Current Consumption
	@16Mhz			21mA		
	@8Mhz			18-20mA			
	@4Mhz			17-18mA		
	@2Mhz			16-17mA	
	@1Mhz			13-16mA		

*/

typedef enum
{
	clock_div_1 = 0,
	clock_div_2 = 1,
	clock_div_4 = 2,
	clock_div_8 = 3,
	clock_div_16 = 4,
	clock_div_32 = 5,
	clock_div_64 = 6,
	clock_div_128 = 7,
	clock_div_256 = 8
} clock_div;

#define clock_prescale_enable() (CLKPR = (uint8_t)(1 << CLKPCE))	// The CLKPCE bit is only updated when the other bits in CLKPR are simultaneously written to zero.
#define clock_prescale_set(x)	(CLKPR = (uint8_t)(x))
#define clock_prescale_get()	(clock_div_t)(CLKPR & (uint8_t)((1<<CLKPS0)|(1<<CLKPS1)|(1<<CLKPS2)|(1<<CLKPS3)))

#endif /* POWER_H_ */
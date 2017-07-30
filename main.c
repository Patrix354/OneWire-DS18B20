#define F_CPU 14745600UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "ds18b20.h"

int main(void)
{
	//declaration necessary variables
	int8_t cel = 0;
	uint8_t cel_fract = 0;
	uint8_t subzero = 0;
	
	//load sensors adresses	 
	search_sensors();

	//init interrupt
	sei();
	
	while(1) 
	{
		for(uint8_t i = 0; i < SENSORS; i++)
		{
			DS18B20_meas_temp(DS18B20_POWER_EXT);	//Delay isn't necessary for this library
			if(DS18B20_read_temp(sensors_ID[i], &subzero, &cel, &cel_fract) == 0)
			{
				//Here you can do something after correct measurement
			}
			else
			{
				//This code will execute if exist any errors in measurement
			}

		}
	}
	
	return 0;
}

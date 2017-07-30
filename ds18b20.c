#define F_CPU 14745600UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "onewire.h"
#include "ds18b20.h"

uint8_t sensors_ID[SENSORS][8];

//
// get sensor address
//
void search_sensors(void)
{
	uint8_t address[8];
	
	cli();
	ow_reset_search();
	for(uint8_t j = 0; j < SENSORS; j++)	//search address
	{
		if(!ow_search(address, 1))
		{
			break;
		}
		if (address[0] != 0x28)	//skip if adress isn't DS18B20 address
		{
			continue;
		}

		if (crc8(address, 7) != address[7])	//checking adress
		{
			break;
		}
		else
		{
			for(uint8_t i = 0; i < 8; i++)	//copying adress into main array
			{
				sensors_ID[j][i] = address[i];
			}
		}
	}
	
	sei();
}

//
// get temperature
//
uint8_t DS18B20_read_temp(uint8_t* ds18b20_address, uint8_t* sub_zero, int8_t* cel, uint8_t* cel_fract)
{
	uint8_t temperature_l;
	uint8_t temperature_h;
	uint32_t temp = 0;
	
	cli();
	
	while(!ow_readbit()); //wait until conversion is complete
	ow_reset(); //ow_reset

	ow_writebyte(DS18B20_CMD_MATCHROM);	//match rom
	for (uint8_t i = 0; i < 8; i++)	//choose correct sensor
	{
		ow_writebyte(ds18b20_address[i]);
	}
	ow_writebyte(DS18B20_CMD_RSCRATCHPAD); //read scratchpad

	//read 2 byte from scratchpad
	temperature_l = ow_readbyte();
	temperature_h = ow_readbyte();
	ow_reset();	//reset

	//convert the 12 bit value obtained
	temp = ( (uint32_t)( ( temperature_h << 8 ) + temperature_l ) * 1000) >> 4;
	*cel = temp / 1000;
	*cel_fract = temp % 10;
	
	//detect error
	if(*cel > 125 || *cel < -55)
	{
		*cel = 0;
		*sub_zero = 0;
		*cel_fract = 0;
		sei();
		return 1;
	}
		
	//invert numbers if temperature is negative 
	if (*cel < 0)
	{
		*cel = (-*cel);
		*cel_fract = (-*cel_fract);
		*sub_zero = 1;
	}
	else 
	{
		*sub_zero = 0;
	}
	
	sei();
	return 0;
}

void DS18B20_meas_temp(uint8_t power_extern)
{	
	cli();
	ow_reset(); //ow_reset
	ow_writebyte(DS18B20_CMD_SKIPROM); //skip ROM
	ow_writebyte(DS18B20_CMD_CONVERTTEMP); //start temperature conversion
	
	if(power_extern)	//if parasite mode
	{
		ow_parasite_enable();
		_delay_ms(750);
	}
	sei();
}
#define F_CPU 14745600UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "onewire.h"

uint8_t ROM_NO[8];
uint8_t LastDiscrepancy;
uint8_t LastFamilyDiscrepancy;
uint8_t LastDeviceFlag;
uint8_t dev_num = 0;

//
//	ow_reset
//
uint8_t ow_reset(void)
{
	uint8_t i;

	//low for 480us
	OW_PIN_LOW(); //low
	OW_PIN_OUT(); //output
	_delay_us(480);

	//release line and wait for 60uS
	OW_PIN_IN(); //input
	_delay_us(60);

	//get value and wait 420us
	i = (~OW_PIN_READ());
	_delay_us(420);

	//return the read value, 0=ok, 1=error
	return i;
}

//
//	write one bit
//
void ow_writebit(uint8_t bit)
{
	//low for 1uS
	OW_PIN_LOW(); //low
	OW_PIN_OUT(); //output
	_delay_us(1);

	//if we want to write 1, release the line (if not will keep low)
	if(bit)
	{
		OW_PIN_IN(); //input
	}

	//wait 60uS and release the line
	_delay_us(60);
	OW_PIN_IN(); //input
}

//
//	read one bit
//
uint8_t ow_readbit(void)
{
	uint8_t bit = 0;

	//low for 1uS
	OW_PIN_LOW(); //low
	OW_PIN_OUT(); //output
	_delay_us(1);

	//release line and wait for 14uS
	OW_PIN_IN(); //input
	_delay_us(14);

	//read the value
	if(OW_PIN_READ())
	{
		bit = 1;
	}

	//wait 45uS and return read value
	_delay_us(45);
	return bit;
}

//
//	write one byte
//
void ow_writebyte(uint8_t byte)
{
	uint8_t i = 8;
	while(i--)
	{
		ow_writebit(byte & 1);
		byte >>= 1;
	}
}

//
//	read one byte
//
uint8_t ow_readbyte(void)
{
	uint8_t i = 8;
	uint8_t n = 0;
	while(i--)
	{
		n >>= 1;
		n |= (ow_readbit() << 7);
	}
	return n;
}

//
//	Read crc8
//
uint8_t crc8(const uint8_t *addr, uint8_t len)
{
	uint8_t crc = 0;
	uint8_t inbyte = 0;

	while (len--) 
	{
		inbyte = *addr++;
		for (uint8_t i = 8; i; i--) {
			uint8_t mix = (crc ^ inbyte) & 0x01;
			crc >>= 1;
			if (mix) crc ^= 0x8C;
			inbyte >>= 1;
		}
	}
	return crc;
}

//
//	ow_reset ow_search stage
//
void ow_reset_search(void)
{
	// ow_reset the ow_search state
	LastDiscrepancy = 0;
	LastDeviceFlag = false;
	LastFamilyDiscrepancy = 0;
	dev_num = 0;
	
	for(int i = 7; ; i--) 
	{
		ROM_NO[i] = 0;
		if (i == 0) 
		{
			break;
		}
	}
}

// Setup the ow_search to find the device type 'family_code' on the next call
// to ow_search(*newAddr) if it is present.
void ow_target_search(uint8_t family_code)
{
	// set the ow_search state to find ow_searchFamily type devices
	ROM_NO[0] = family_code;
	for (uint8_t i = 1; i < 8; i++)
	{
		ROM_NO[i] = 0;
		LastDiscrepancy = 64;
		LastFamilyDiscrepancy = 0;
		LastDeviceFlag = false;
	}
}

// ow_search address fo devices on onewire bus
// Return TRUE  : device found, ROM number in ROM_NO buffer
//        FALSE : device not found, end of ow_search
//
uint8_t ow_search(uint8_t *newAddr, uint8_t ow_search_mode)
{
	uint8_t id_bit_number = 1;
	uint8_t last_zero = 0;
	uint8_t rom_byte_number = 0;
	uint8_t ow_search_result = 0;
	uint8_t rom_byte_mask = 1;
	uint8_t id_bit;
	uint8_t cmp_id_bit;
	uint8_t ow_search_direction;

	// if the last call was not the last one
	
	if (!LastDeviceFlag)
	{
		// 1-Wire ow_reset
		if(!ow_reset())
		{
			// ow_reset the ow_search
			LastDiscrepancy = 0;
			LastDeviceFlag = false;
			LastFamilyDiscrepancy = 0;
			return false;
		}
		// issue the ow_search command
		if (ow_search_mode == 1)
		{
			ow_writebyte(0xF0);   // NORMAL ow_search
		}
		else
		{
			ow_writebyte(0xEC);   // CONDITIONAL ow_search
		}

		// loop to do the ow_search
		do
		{
			// read a bit and its complement
			id_bit = ow_readbit();
			cmp_id_bit = ow_readbit();

			// check for no devices on 1-wire
			if ((id_bit == 1) && (cmp_id_bit == 1))
			{
				break;
			}
			else
			{
				// all devices coupled have 0 or 1
				if (id_bit != cmp_id_bit)
				{
					ow_search_direction = id_bit;  // bit write value for ow_search
				}
				else
				{
					// if this discrepancy if before the Last Discrepancy
					// on a previous next then pick the same as last time
					if (id_bit_number < LastDiscrepancy)
					{
						ow_search_direction = ((ROM_NO[rom_byte_number] & rom_byte_mask) > 0);
					}
					else
					{
						// if equal to last pick 1, if not then pick 0
						ow_search_direction = (id_bit_number == LastDiscrepancy);
					}
	
					// if 0 was picked then record its position in LastZero
					if (ow_search_direction == 0)
					{
						last_zero = id_bit_number;

						// check for Last discrepancy in family
						if (last_zero < 9)
						{
							LastFamilyDiscrepancy = last_zero;
						}
					}
				}

					// set or clear the bit in the ROM byte rom_byte_number
					// with mask rom_byte_mask
				if (ow_search_direction == 1)
				{
					ROM_NO[rom_byte_number] |= rom_byte_mask;
				}
				else
				{
					ROM_NO[rom_byte_number] &= ~rom_byte_mask;
				}
				// serial number ow_search direction write bit
				ow_writebit(ow_search_direction);
				// increment the byte counter id_bit_number
				// and shift the mask rom_byte_mask
				id_bit_number++;
				rom_byte_mask <<= 1;

				// if the mask is 0 then go to new SerialNum byte rom_byte_number and ow_reset mask
				if (rom_byte_mask == 0)
				{
					rom_byte_number++;
					rom_byte_mask = 1;
				}
			}
		}
		while(rom_byte_number < 8);  // loop until through all ROM bytes 0-7
		// if the ow_search was successful then
		if (!(id_bit_number < 65))
		{
			// ow_search successful so set LastDiscrepancy,LastDeviceFlag,ow_search_result
			LastDiscrepancy = last_zero;

			// check for last device
			if (LastDiscrepancy == 0)
			LastDeviceFlag = true;

			ow_search_result = true;
		}
	}

	// if no device found then ow_reset counters so next 'ow_search' will be like a first
	if (!ow_search_result || !ROM_NO[0])
	{
		LastDiscrepancy = 0;
		LastDeviceFlag = false;
		LastFamilyDiscrepancy = 0;
		ow_search_result = false;
	}
	else
	{
		for (int i = 0; i < 8; i++)
		{
			newAddr[i] = ROM_NO[i];
		}
	}
	return ow_search_result;
}

void ow_select(const uint8_t rom[8])
{
	uint8_t i;

	ow_writebyte(0x55);	// Choose ROM
	for (i = 0; i < 8; i++)
	{
		ow_writebyte(rom[i]);	// Send rom byte
	}
}

//
// Do a ROM skip
//
void ow_skip(void)
{
	ow_writebyte(0xCC);	// Skip ROM
}

void ow_parasite_enable(void)	// Enable paraside mode
{
	OW_PIN_HI();
	OW_PIN_OUT();
}

void ow_parasite_disable(void)	// Disable paraside mode
{
	OW_PIN_LOW();
	OW_PIN_IN();
}
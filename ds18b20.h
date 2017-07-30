#ifndef DS18B20_H_
#define DS18B20_H_

//commands
#define DS18B20_CMD_CONVERTTEMP 0x44
#define DS18B20_CMD_RSCRATCHPAD 0xBE
#define DS18B20_CMD_WSCRATCHPAD 0x4E
#define DS18B20_CMD_CPYSCRATCHPAD 0x48
#define DS18B20_CMD_RECEEPROM 0xB8
#define DS18B20_CMD_RPWRSUPPLY 0xB4
#define DS18B20_CMD_SEARCHROM 0xF0
#define DS18B20_CMD_READROM 0x33
#define DS18B20_CMD_MATCHROM 0x55
#define DS18B20_CMD_SKIPROM 0xCC
#define DS18B20_CMD_ALARMSEARCH 0xEC

#define DS18B20_POWER_EXT 0
#define DS18B20_POWER_PARASITE 1

//amount of sensors
#define SENSORS 1

//extern definitions
extern uint8_t sensors_ID[SENSORS][8];

//functions
uint8_t DS18B20_read_temp(uint8_t* ds18b20_address, uint8_t* sub_zero, int8_t* cel, uint8_t* cel_fract);
void DS18B20_meas_temp(uint8_t power_extern);
void search_sensors(void);

#endif

#ifndef ONEWIRE_H_
#define ONEWIRE_H_

//bool values
#ifndef false
#define false 0
#endif

#ifndef true
#define true 1
#endif

//setup connection
#define OW_PORT PORTB
#define OW_DDR DDRB
#define OW_IN PINB
#define OW_PIN PORTB0

#define OW_PIN_HI()	OW_PORT |= (1<<OW_PIN)
#define OW_PIN_LOW() OW_PORT &= ~ (1<<OW_PIN);
#define OW_PIN_OUT() OW_DDR |= (1<<OW_PIN);
#define OW_PIN_IN() OW_DDR &= ~(1<<OW_PIN);
#define OW_PIN_READ() OW_IN & (1<<OW_PIN)

uint8_t ow_reset(void);
void ow_writebit(uint8_t bit);
uint8_t ow_readbit(void);
void ow_writebyte(uint8_t byte);
uint8_t ow_readbyte(void);
uint8_t crc8(const uint8_t *addr, uint8_t len);
void ow_reset_search(void);
void ow_target_search(uint8_t family_code);
uint8_t ow_search(uint8_t *newAddr, uint8_t ow_search_mode);
void ow_select(const uint8_t rom[8]);
void ow_skip(void);
void ow_parasite_enable(void);
void ow_parasite_disable(void);

#endif /* ONEWIRE_H_ */
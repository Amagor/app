/**
 * @file PlatformHardware.c
 * Common hardware functions implementation source file.
 * @author Nezametdinov I.E.
 */

#include "../../PIL/Hardware.h"
#include <avr/interrupt.h>

/*******************************************************************************//**
 * @implements InitHardware
 **********************************************************************************/
RESULT InitHardware(void)
{
	// enable external RAM
	MCUCR |= 1<<SRE;
	return SUCCESS;
}


//Вычисляет контрольную сумму CRC8
unsigned char OW_ComputeCRC8(unsigned char inData, unsigned char seed)
{
    unsigned char bitsLeft;
    unsigned char temp;

    for (bitsLeft = 8; bitsLeft > 0; bitsLeft--)
    {
        temp = ((seed ^ inData) & 0x01);
        if (temp == 0)
        {
            seed >>= 1;
        }
        else
        {
            seed ^= 0x18;
            seed >>= 1;
            seed |= 0x80;
        }
        inData >>= 1;
    }
    return seed;
}

//Сброс шины 1-wire
uint8_t OW_reset(void)
{
  uint8_t answer;
  //port B input, PB0 set 0
  OW_PORT &= ~(1<<OW_DQ); //set 0 in PORTB, bit PB0
  OW_DDR |= (1<<OW_DQ); // set 0 in DDRB line DQ -> 1
  _delay_us(480);

  //TRANSMIT RESET PULSE.
  OW_DDR &=~(1<<OW_DQ); //set 1 in DDRB line DQ -> 0
  _delay_us(70);

//RECEIVE RESET PULSE AND DETECT PRESENCE PULSE.
  answer = (OW_PIN & (1<<OW_DQ));
// Complete the reset sequence recovery
  _delay_us(410);
  return (answer); //0-есть устройства, 1-нет устройств
 }

//Запись бита
void OW_write_bit(uint8_t bit)
{
  OW_PORT &= ~(1<<OW_DQ); //set 0 in PORTB, bit PB0
  OW_DDR |= (1<<OW_DQ); //set 0 in DDRB, line DQ -> 1
  _delay_us(10);
    //Send 0 to 1wire (bit == 0)
  if(bit) OW_DDR&=~(1<<OW_DQ);
  _delay_us(70);
  OW_DDR&=~(1<<OW_DQ);
}

//Чтение бита
 uint8_t OW_read_bit(void)
{
  uint8_t result;
  OW_PORT &= ~(1<<OW_DQ); //set 0 in PORTB, bit PB0
  OW_DDR |= (1<<OW_DQ); //set 0 in DDRB line DQ -> 1
  __asm("nop");
  __asm("nop");
  __asm("nop");
  __asm("nop");
  __asm("nop");
  __asm("nop");
  OW_DDR&=~(1<<OW_DQ);
  _delay_us(10);

  result = (OW_PIN & (1<<OW_DQ)); //check  1st pin ( PB0 )

  return result;
}

//Запись байта
void OW_write_byte(uint8_t command)
{
    for (uint8_t i = 0; i < 8; i++)
     {
         OW_write_bit(command & 0x01); //send one bit
         command >>= 1; //shift to next bit
     }
	_delay_us(120);
}

//Чтение байта
uint8_t OW_read_byte(void)
{
  unsigned char i;
  unsigned char value = 0;
  /*~~~~~~~~~~~~~~~~~~~~~~*/

  for(i = 0; i < 8; i++)
  {
    if(OW_read_bit())
	  value |= 0x01 << i;/* читаем один бит и записываем его в разряд i */

    /* ожидаем окончания временного интервала */
	_delay_us(120);
  }
  return(value);
}


MAC_EXTENDED_ADDR MACLayer_GetHWAddr(void)
{
//	MAC_EXTENDED_ADDR HWAddr;
	int i;
	uint8_t *c;
	
	for(i=0;i<8;i++)
	{
	c=((uint8_t*)&HWAddr)+i;
	*c=MAC[i];
	}
	return HWAddr;
}

void read_MAC()
{
    uint8_t crc = 0;
    // initiate array to 0
    for (uint8_t a1 = 0; a1 < 8; a1++) MAC[a1] = 0;
 	OW_reset();
    OW_write_byte(READ_ROM); // 0x33
    //OW_write_byte(READ_SCRATCHPAD); // read scratchpad 0xBE
 //Recive data  in 1 Wire
    for (uint8_t c1 = 0; c1 < 8; c1++)
    {
        MAC[c1] = OW_read_byte();
    }
 //Check CRC8  to 1 Wire data
    for(uint8_t i = 0; i < 7; i++)
      {
         crc =  OW_ComputeCRC8(MAC[i], crc);
       }
       if (crc == MAC[7])
       {
        MACLayer_GetHWAddr();
		return;
        }
		MACLayer_GetHWAddr();
		return;
  //return ( ERROR_CRC); // Message -80
}  
  


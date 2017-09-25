#ifndef SPI_H_
#define SPI_H_

#include "msp430f2272.h"
#include "define.h"

//==================================主出从入====================================
#define MOSI_0 P3OUT &= ~BIT1
#define MOSI_1 P3OUT |= BIT1
//===================================SPI时钟端口================================
#define SCK_0 P3OUT &= ~BIT3
#define SCK_1 P3OUT |= BIT3
//===================================主入从出===================================
#define MISO P3IN & BIT2

INT8U SPI_ExchangeByte(INT8U data);
/*INT8U spi_read_byte(void);
void spi_write_byte(INT8U data);*/

#endif /*SPI_H_*/

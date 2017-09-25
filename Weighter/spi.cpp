#include "spi.h"
/* MOSI  P3.1
 * MISO  P3.2
 * SCLK	 P3.3
*/
INT8U SPI_ExchangeByte(INT8U data){
	INT8U i,temp;
	
	SCK_0 ;
	for(i = 0; i < 8; i++){
		if(data&0x80){
			MOSI_1;
		}
		else{
			MOSI_0;
		}
		data <<= 1;
		SCK_1;
		temp <<= 1;
		if(MISO) temp++;  //MISO
		SCK_0 ;
	}
	return temp;
}

/*
INT8U spi_read_byte(void){
	INT8U data;
	
	IFG2 &= ~UCB0RXIFG;
	//P3OUT &= ~0x01;                         // Enable , /CS reset
    UCB0TXBUF = 0x00;                       // Dummy write to start SPI
    while (!(IFG2 & UCB0RXIFG));            // USCI_B0 RX buffer ready?
    data = UCB0RXBUF;                       // data = 00|DATA
    //P3OUT |= 0x01;                          // Disable , /CS set	
	return data;
}

void spi_write_byte(INT8U data){
	
	while (!(IFG2 & UCA0TXIFG));
	UCB0TXBUF = data;
}
INT8U SPI_ExchangeByte(INT8U data){
	//INT16U delay_times_times = 0;
	INT8U ret_data;
	
    while (!(IFG2 & UCB0TXIFG)){            // USCI_A0 TX buffer ready?
    	//if(delay_times++ >  200){
    	//	break;	
    	//}
    }
    UCB0TXBUF = data;                       // Byte to SPI TXBUF
		
	IFG2 &= ~UCB0RXIFG;                     // Clear int flag
    UCB0TXBUF = 0x00;                       // Dummy write to start SPI
    //delay_times = 0;
    while (!(IFG2 & UCB0RXIFG)){            // RXBUF ready?
    	//if(delay_times++ > 200){
    	//	break;	
    	//}
    }
    ret_data = UCB0RXBUF;                       // Move value
    return ret_data;
}*/

#include "msp430f2272.h"
#include "define.h"

INT8U WriteFlash(INT16U addr, INT8U *buf, INT8U len)//
{
	INT8U reg1 = IE1;
	INT8U reg2 = IE2;
	
	_DINT();  // diable interrupts
	
	while (FCTL3 & BUSY);  // wait for the end of previous operation
	
  	FCTL3 = FWKEY;  // Clear Lock bit
  	FCTL1 = FWKEY + WRT;  // Set WRT bit for write operation

  	for (INT8U i = 0; i < len; i++)
  	{
    	*(INT8U*)addr++ = buf[i];  // Write value to flash
  	}

	while (FCTL3 & BUSY);  // wait for the end of current operation
	
  	FCTL1 = FWKEY;  // Clear WRT bit
  	FCTL3 = FWKEY + LOCK;  // Set LOCK bit
  	
  	_EINT();  // enable interrupts
  	
  	IE1 = reg1;
  	IE2 = reg2;
  	
  	return (FCTL3 & FAIL);  // return 0 for success, 1 otherwise
}

void ReadFlash(INT16U addr, INT8U *buf, INT8U len)
{
	for(INT8U i = 0; i < len; i++)
	{
    	buf[i] = *(INT8U*)addr++;
	}

	return;
}

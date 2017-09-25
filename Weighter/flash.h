#ifndef FLASH_H_
#define FLASH_H_

int ReadFlash(INT16U addr, INT8U *buf, INT8U len);
int WriteFlash(INT16U addr, INT8U *buf, INT8U len);

#endif /*FLASH_H_*/


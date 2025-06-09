#ifndef _SC_IO_H_
#define _SC_IO_H_

//#include <linux/io.h>
#include <linux/types.h>
#include "main.h"

uint32_t    SC_In32(uint64_t addr);
void        SC_Out32(uint64_t addr, uint32_t val);

#endif 

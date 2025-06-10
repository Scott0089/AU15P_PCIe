#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>

#undef __LINUX__

//#define SDT

#include "sc_io.h"

#include "xsysmon.h"
#include "xgpio.h"
#include "xtmrctr.h"
#include "xv_tpg.h"
#include "xvidc.h"

#define TARGET_PHYS_ADDR 0x40000000 

extern int fd;

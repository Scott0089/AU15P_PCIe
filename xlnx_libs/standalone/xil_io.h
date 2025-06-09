/******************************************************************************
* Xil_IO uses microblaze outputs, we must replace these macros with Litepcie writes,
* such that all existing drivers simply use this instead. Configure the file pointer in our main script for the library.
*
******************************************************************************/

#ifndef XIL_IO_H           /**< prevent circular inclusions */
#define XIL_IO_H           /**< by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/


#include "sc_io.h"

#define Xil_In32(addr)    			SC_In32(addr)
#define Xil_Out32(addr, data)  	 	SC_Out32(addr, data)



#endif /* end of protection macro */
/**
* @} End of "addtogroup common_io_interfacing_apis".
*/



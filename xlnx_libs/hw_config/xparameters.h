#ifndef XPARAMETERS_H   /* prevent circular inclusions */
#define XPARAMETERS_H   /* by using protection macros */

//Note this file is modified from the Microblaze to work on Linux with our custom build. 
//All configurations save for address are updated, all MCU based defines are removed

/******************************************************************/

// No longer need STDIN and STOUT, trace compile errors from here/
//#define STDIN_BASEADDRESS 0x40600000
//#define STDOUT_BASEADDRESS 0x40600000

/******************************************************************/

/* Platform specific definitions */
//#define PLATFORM_RISCV
 
/* Definitions for sleep timer configuration */
//#define XSLEEP_TIMER_IS_DEFAULT_TIMER

/******************************************************************/

#define XPAR_XGPIO_NUM_INSTANCES 3

/* Definitions for peripheral AXI_GPIO_0 */
#define XPAR_AXI_GPIO_0_COMPATIBLE "xlnx,axi-gpio-2.0"
#define XPAR_AXI_GPIO_0_BASEADDR 0x40000000
#define XPAR_AXI_GPIO_0_HIGHADDR 0x4000ffff
#define XPAR_AXI_GPIO_0_INTERRUPT_PRESENT 0x0
#define XPAR_AXI_GPIO_0_IS_DUAL 0x0
#define XPAR_AXI_GPIO_0_GPIO_WIDTH 0x1

/* Canonical definitions for peripheral AXI_GPIO_0 */
#define XPAR_XGPIO_0_BASEADDR 0x40000000
#define XPAR_XGPIO_0_HIGHADDR 0x4000ffff
#define XPAR_XGPIO_0_COMPATIBLE "xlnx,axi-gpio-2.0"
#define XPAR_XGPIO_0_GPIO_WIDTH 0x1
#define XPAR_XGPIO_0_INTERRUPT_PRESENT 0x0
#define XPAR_XGPIO_0_IS_DUAL 0x0

/* Definitions for peripheral AXI_GPIO_1 */
#define XPAR_AXI_GPIO_1_COMPATIBLE "xlnx,axi-gpio-2.0"
#define XPAR_AXI_GPIO_1_BASEADDR 0x40010000
#define XPAR_AXI_GPIO_1_HIGHADDR 0x4001ffff
#define XPAR_AXI_GPIO_1_INTERRUPT_PRESENT 0x0
#define XPAR_AXI_GPIO_1_IS_DUAL 0x1
#define XPAR_AXI_GPIO_1_GPIO_WIDTH 0x1

/* Canonical definitions for peripheral AXI_GPIO_1 */
#define XPAR_XGPIO_1_BASEADDR 0x40010000
#define XPAR_XGPIO_1_HIGHADDR 0x4001ffff
#define XPAR_XGPIO_1_COMPATIBLE "xlnx,axi-gpio-2.0"
#define XPAR_XGPIO_1_GPIO_WIDTH 0x1
#define XPAR_XGPIO_1_INTERRUPT_PRESENT 0x0
#define XPAR_XGPIO_1_IS_DUAL 0x1

/* Definitions for peripheral TPG_RESET */
#define XPAR_AXI_TPG_RESET_COMPATIBLE "xlnx,axi-gpio-2.0"
#define XPAR_AXI_TPG_RESET_BASEADDR 0x40050000
#define XPAR_AXI_TPG_RESET_HIGHADDR 0x4005ffff
#define XPAR_AXI_TPG_RESET_INTERRUPT_PRESENT 0x0
#define XPAR_AXI_TPG_RESET_IS_DUAL 0x0
#define XPAR_AXI_TPG_RESET_GPIO_WIDTH 0x1

/* Canonical definitions for peripheral TPG_RESET */
#define XPAR_XTPG_RESET_BASEADDR 0x40050000
#define XPAR_XTPG_RESET_HIGHADDR 0x4005ffff
#define XPAR_XTPG_RESET_COMPATIBLE "xlnx,axi-gpio-2.0"
#define XPAR_XTPG_RESET_GPIO_WIDTH 0x1
#define XPAR_XTPG_RESET_INTERRUPT_PRESENT 0x0
#define XPAR_XTPG_RESET_IS_DUAL 0x0

#define XPAR_XSYSMON_NUM_INSTANCES 1

/* Definitions for peripheral SYSTEM_MANAGEMENT_WIZ_0 */
#define XPAR_SYSTEM_MANAGEMENT_WIZ_0_COMPATIBLE "xlnx,system-management-wiz-1.3"
#define XPAR_SYSTEM_MANAGEMENT_WIZ_0_BASEADDR 0x40030000
#define XPAR_SYSTEM_MANAGEMENT_WIZ_0_HIGHADDR 0x4003ffff
#define XPAR_SYSTEM_MANAGEMENT_WIZ_0_INCLUDE_INTR 0x1
#define XPAR_SYSTEM_MANAGEMENT_WIZ_0_IP_TYPE 0x1

/* Canonical definitions for peripheral SYSTEM_MANAGEMENT_WIZ_0 */
#define XPAR_XSYSMON_0_BASEADDR 0x40030000
#define XPAR_XSYSMON_0_HIGHADDR 0x4003ffff
#define XPAR_XSYSMON_0_COMPATIBLE "xlnx,system-management-wiz-1.3"
#define XPAR_XSYSMON_0_INCLUDE_INTR 0x1
#define XPAR_XSYSMON_0_IP_TYPE 0x1

#define XPAR_XTMRCTR_NUM_INSTANCES 1

/* Definitions for peripheral AXI_TIMER_0 */
#define XPAR_AXI_TIMER_0_COMPATIBLE "xlnx,axi-timer-2.0"
#define XPAR_AXI_TIMER_0_BASEADDR 0x40020000
#define XPAR_AXI_TIMER_0_HIGHADDR 0x4002ffff
#define XPAR_AXI_TIMER_0_CLOCK_FREQUENCY 0x5f5e100

/* Canonical definitions for peripheral AXI_TIMER_0 */
#define XPAR_XTMRCTR_0_BASEADDR 0x40020000
#define XPAR_XTMRCTR_0_HIGHADDR 0x4002ffff
#define XPAR_XTMRCTR_0_COMPATIBLE "xlnx,axi-timer-2.0"
#define XPAR_XTMRCTR_0_CLOCK_FREQUENCY 0x5f5e100

#define XPAR_XV_TPG_NUM_INSTANCES 1

/* Definitions for peripheral V_TPG_0 */
#define XPAR_V_TPG_0_COMPATIBLE "xlnx,v-tpg-8.2"
#define XPAR_V_TPG_0_BASEADDR 0x40060000
#define XPAR_V_TPG_0_HIGHADDR 0x4006ffff
#define XPAR_V_TPG_0_AXI4S_SLAVE 0x0
#define XPAR_V_TPG_0_SAMPLES_PER_CLOCK 0x2
#define XPAR_V_TPG_0_NUM_VIDEO_COMPONENTS 0x3
#define XPAR_V_TPG_0_MAX_COLS 0x1000
#define XPAR_V_TPG_0_MAX_ROWS 0x870
#define XPAR_V_TPG_0_MAX_DATA_WIDTH 0xa
#define XPAR_V_TPG_0_SOLID_COLOR 0x1
#define XPAR_V_TPG_0_RAMP 0x1
#define XPAR_V_TPG_0_COLOR_BAR 0x1
#define XPAR_V_TPG_0_DISPLAY_PORT 0x1
#define XPAR_V_TPG_0_COLOR_SWEEP 0x1
#define XPAR_V_TPG_0_ZONE_PLATE 0x1
#define XPAR_V_TPG_0_FOREGROUND 0x1

/* Canonical definitions for peripheral V_TPG_0 */
#define XPAR_XV_TPG_0_BASEADDR 0x40060000
#define XPAR_XV_TPG_0_HIGHADDR 0x4006ffff
#define XPAR_XV_TPG_0_AXI4S_SLAVE 0x0
#define XPAR_XV_TPG_0_COMPATIBLE "xlnx,v-tpg-8.2"
#define XPAR_XV_TPG_0_COLOR_BAR 0x1
#define XPAR_XV_TPG_0_COLOR_SWEEP 0x1
#define XPAR_XV_TPG_0_DISPLAY_PORT 0x1
#define XPAR_XV_TPG_0_FOREGROUND 0x1
#define XPAR_XV_TPG_0_MAX_COLS 0x1000
#define XPAR_XV_TPG_0_MAX_ROWS 0x870
#define XPAR_XV_TPG_0_MAX_DATA_WIDTH 0xa
#define XPAR_XV_TPG_0_NUM_VIDEO_COMPONENTS 0x3
#define XPAR_XV_TPG_0_RAMP 0x1
#define XPAR_XV_TPG_0_SAMPLES_PER_CLOCK 0x2
#define XPAR_XV_TPG_0_SOLID_COLOR 0x1
#define XPAR_XV_TPG_0_ZONE_PLATE 0x1

#endif  /* end of protection macro */

#include "main.h"
#include "xparameters.h"

#define WHITE 0x07
#define CYAN 0x06
#define MAGENTA 0x05
#define BLUE 0x04
#define YELLOW 0x03
#define GREEN 0x02
#define RED 0x01
#define OFF 0x00

XGpio gpioInst[2];
XTmrCtr tmrInst;
XSysMon sysmonInst;
XSysMon_Config *ConfigPtr;

int fd = -1;

typedef struct
{
    uint32_t TempRawData;
    uint32_t VCCINTRawData;
    uint32_t VCCAUXRawData;
    uint32_t VCCBRAMRawData;
    uint32_t VUSR0RawData;
    uint32_t VUSR1RawData;
    uint32_t VUSR2RawData;
    
    float TempData;
    float VCCINTData;
    float VCCAUXData;
    float VCCBRAMData;
    float VUSR0Data;
    float VUSR1Data;
    float VUSR2Data;

} SYSMON_Data;


SYSMON_Data xadcInst;

int SysMonFractionToInt(float FloatNum)
{
    float Temp;

    Temp = FloatNum;
    if(FloatNum < 0)
    {
        Temp = -(FloatNum);
    }

    return (((int)((Temp - (float)((int)Temp)) * (1000.0f))));
}

int main()
{
    uint32_t status;

    fd = open("/dev/xdma0_user", O_RDWR);
    if (fd < 0)
    {
        printf("Failed to open /dev/xdma0_user!\r\n");
        printf("Check if Driver is installed. \r\n");
        return XST_FAILURE;
    }

    printf("\n\n\r     ** AU15P PCIe Test **\n\r");   
    
    status = XGpio_Initialize(&gpioInst[0], XPAR_AXI_GPIO_0_BASEADDR);
    if(status != XST_SUCCESS)
    {
        printf("Failed to Init RED LED! \r\n");
        return XST_FAILURE;
    }

    status = XGpio_Initialize(&gpioInst[1], XPAR_AXI_GPIO_1_BASEADDR);
    if(status != XST_SUCCESS)
    {
        printf("Failed to Init RGB LEDs! \r\n");
        return XST_FAILURE;
    }

    ConfigPtr = XSysMon_LookupConfig(XPAR_XSYSMON_0_BASEADDR);
    if(ConfigPtr == NULL)
    {
        printf("Failed to get Sysmon Config!\r\n");
        return XST_FAILURE;
    }

    status = XSysMon_CfgInitialize(&sysmonInst, ConfigPtr, ConfigPtr->BaseAddress);
    if(status != XST_SUCCESS)
    {
        printf("Failed to Init Sysmon! \r\n");
        return XST_FAILURE;
    }
    
    status = XTmrCtr_Initialize(&tmrInst, XPAR_AXI_TIMER_0_BASEADDR);
    if(status != XST_SUCCESS)
    {
        printf("Failed to Init Timer! \r\n");
        return XST_FAILURE;
    }

    XTmrCtr_SetResetValue(&tmrInst, 0, (u32)-50000000);
    XTmrCtr_SetResetValue(&tmrInst, 1, (u32)-7500000);
    XTmrCtr_Start(&tmrInst, 0);
    XTmrCtr_Start(&tmrInst, 1);

    uint32_t read_buff[2];
    uint32_t write_buff[2];

    while(1)
    {
        xadcInst.TempRawData = XSysMon_GetAdcData(&sysmonInst, XSM_CH_TEMP);
        xadcInst.VCCINTRawData = XSysMon_GetAdcData(&sysmonInst, XSM_CH_VCCINT);
        xadcInst.VCCAUXRawData = XSysMon_GetAdcData(&sysmonInst, XSM_CH_VCCAUX);
        xadcInst.VCCBRAMRawData = XSysMon_GetAdcData(&sysmonInst, XSM_CH_VBRAM);

        xadcInst.VUSR0RawData = XSysMon_GetAdcData(&sysmonInst, XSM_CH_VUSR0);
        xadcInst.VUSR1RawData = XSysMon_GetAdcData(&sysmonInst, XSM_CH_VUSR1);
        xadcInst.VUSR2RawData = XSysMon_GetAdcData(&sysmonInst, XSM_CH_VUSR2);
        
        xadcInst.TempData = XSysMon_RawToTemperature(xadcInst.TempRawData);
        xadcInst.VCCINTData = XSysMon_RawToVoltage(xadcInst.VCCINTRawData);
        xadcInst.VCCAUXData = XSysMon_RawToVoltage(xadcInst.VCCAUXRawData);
        xadcInst.VCCBRAMData = XSysMon_RawToVoltage(xadcInst.VCCBRAMRawData);

        xadcInst.VUSR0Data = XSysMon_RawToVoltage(xadcInst.VUSR0RawData);
        xadcInst.VUSR1Data = XSysMon_RawToVoltage(xadcInst.VUSR1RawData);
        xadcInst.VUSR2Data = XSysMon_RawToVoltage(xadcInst.VUSR2RawData);

        if(XTmrCtr_IsExpired(&tmrInst, 0))
        {
            XTmrCtr_Reset(&tmrInst, 0);

            read_buff[1] = XGpio_DiscreteRead(&gpioInst[1], 0x01);
            write_buff[1] = (read_buff[1] == 0x01) ? 0x07: read_buff[1] - 1;
            XGpio_DiscreteWrite(&gpioInst[1], 0x02, write_buff[1]);

            printf("\r\nTemperature: %0d.%03d C \r\n", (int)xadcInst.TempData, SysMonFractionToInt(xadcInst.TempData));
            printf("VCCINT: %0d.%03d V \r\n", (int)xadcInst.VCCINTData, SysMonFractionToInt(xadcInst.VCCINTData));
            printf("VCCAUX: %0d.%03d V \r\n", (int)xadcInst.VCCAUXData, SysMonFractionToInt(xadcInst.VCCAUXData));
            printf("VCCBRAM: %0d.%03d V \r\n", (int)xadcInst.VCCBRAMData, SysMonFractionToInt(xadcInst.VCCBRAMData));

            printf("BANK 64 VCCO: %0d.%03d V \r\n", (int)xadcInst.VUSR0Data, SysMonFractionToInt(xadcInst.VUSR0Data));
            printf("MGTAVCC: %0d.%03d V \r\n", (int)xadcInst.VUSR1Data, SysMonFractionToInt(xadcInst.VUSR1Data));
            printf("MGTAVTT: %0d.%03d V \r\n", (int)xadcInst.VUSR2Data, SysMonFractionToInt(xadcInst.VUSR2Data));

        }

        if(XTmrCtr_IsExpired(&tmrInst, 1))
        {
            XTmrCtr_Reset(&tmrInst, 1);

            read_buff[0] = XGpio_DiscreteRead(&gpioInst[1], 0x01);
            write_buff[0] = (read_buff[0] == 0x01) ? 0x07: read_buff[0] - 1;
            XGpio_DiscreteWrite(&gpioInst[1], 0x01, write_buff[0]);
        }
    } 
  
    printf("\nEverything done! \r\n Exiting... \r\n");

    close(fd);

    return 0;
}

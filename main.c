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

#define FRAME_WIDTH 1920
#define FRAME_HEIGHT 1080
#define BYTES_PER_PIXEL 3
#define FRAME_SIZE (FRAME_WIDTH * FRAME_HEIGHT * BYTES_PER_PIXEL)
#define DMA_ALIGNMENT 32

#define SHM_NAME "/xdma_buffer"

XGpio gpioInst[2];
XTmrCtr tmrInst;
XSysMon sysmonInst;
XSysMon_Config *ConfigPtr;

XV_tpg_Config *TpgPtr;
XV_tpg tpgInst;

XGpio tpgresetInst;

int fd = -1;
int fd_read = -1;

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

static inline uint8_t convert_10bit_to_8bit(uint16_t val10)
{
    //return ((val10 * 255) / 1023);
    return val10 >> 2;
}

int tpg_reset()
{
    XGpio_DiscreteWrite(&tpgresetInst, 0x01, 0x00);
    usleep(300000);
    XGpio_DiscreteWrite(&tpgresetInst, 0x01, 0x01);

    return XST_SUCCESS;
}

int tpg()
{
    uint32_t status;

    TpgPtr = XV_tpg_LookupConfig(XPAR_V_TPG_0_BASEADDR);
    if (TpgPtr == NULL)
    {
        printf("TPG failed to get config! \r\n");
        return XST_FAILURE;
    }
    status = XV_tpg_CfgInitialize(&tpgInst, TpgPtr, TpgPtr->BaseAddress);
    if (status != XST_SUCCESS)
    {
        printf("TPG Failed to Init! \r\n");
        return XST_FAILURE;
    }

    tpg_reset();

    printf("TPG Init Success\r\n");

    XV_tpg_Set_height(&tpgInst, FRAME_HEIGHT);
    XV_tpg_Set_width(&tpgInst, FRAME_WIDTH);
    XV_tpg_Set_colorFormat(&tpgInst, XVIDC_CSF_RGB);
    XV_tpg_Set_maskId(&tpgInst, 0);
    XV_tpg_Set_motionSpeed(&tpgInst, 5);
    XV_tpg_Set_motionEn(&tpgInst, 1);
    XV_tpg_Set_bckgndId(&tpgInst, XTPG_BKGND_SOLID_GREEN);

    
    XV_tpg_Set_boxColorB(&tpgInst, 0xFF);
    XV_tpg_Set_boxColorR(&tpgInst, 0x00);
    XV_tpg_Set_boxColorG(&tpgInst, 0x00);
    XV_tpg_Set_boxSize(&tpgInst, 50);
    
    XV_tpg_Set_ovrlayId(&tpgInst, 0x01);

    printf("Finished Config\r\n");

    XV_tpg_EnableAutoRestart(&tpgInst);
    XV_tpg_Start(&tpgInst);

    printf("TPG Started\r\n");

    // printf("Data: %x \r\n", XV_tpg_ReadReg(XPAR_V_TPG_0_BASEADDR, XV_TPG_CTRL_ADDR_AP_CTRL));

    // printf("Resetting TPG! \r\n");
    // printf("Done resetting TPG\r\n");
    // while(!XV_tpg_IsIdle(&tpgInst));

    return XST_SUCCESS;
}

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

int streaming2() { //This is all ChatGPT. Need to reverse engineer this whole shared memory thing to figure out why it works.
    ssize_t bytes_read, total_read;
    size_t padded_width = ((FRAME_WIDTH + 7) / 8) * 8;
    size_t stride_2pixels = padded_width / 2;
    size_t stride_bytes = stride_2pixels * 8;
    size_t frame_bytes = stride_bytes * FRAME_HEIGHT;

    uint8_t* input_buffer = malloc(frame_bytes);
    if (!input_buffer) {
        printf("Malloc failed for input_buffer\n");
        return XST_FAILURE;
    }

    // Create shared memory
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd < 0) {
        perror("shm_open");
        free(input_buffer);
        return XST_FAILURE;
    }

    // Size of shared memory: 4 bytes (control) + RGB frame
    size_t shm_size = sizeof(uint32_t) + FRAME_SIZE;
    if (ftruncate(shm_fd, shm_size) == -1) {
        perror("ftruncate");
        close(shm_fd);
        free(input_buffer);
        return XST_FAILURE;
    }

    uint8_t* shm_ptr = mmap(NULL, shm_size, PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        perror("mmap");
        close(shm_fd);
        free(input_buffer);
        return XST_FAILURE;
    }

    uint32_t* frame_counter = (uint32_t*)shm_ptr;
    uint8_t* output_buffer = shm_ptr + sizeof(uint32_t);

    int fd_read = open("/dev/xdma0_c2h_0", O_RDONLY);
    if (fd_read < 0) {
        printf("Failed to open DMA device\n");
        munmap(shm_ptr, shm_size);
        close(shm_fd);
        free(input_buffer);
        return XST_FAILURE;
    }

    while(1)
    {
        total_read = 0;
            while (total_read < frame_bytes) {
                //bytes_read = pread(fd_read, input_buffer + total_read, frame_bytes - total_read, total_read);
                bytes_read = read(fd_read, input_buffer + total_read, frame_bytes - total_read);
                if (bytes_read < 0) {
                    printf("Failed to read from DMA\n");
                    munmap(shm_ptr, shm_size);
                    close(shm_fd);
                    free(input_buffer);
                    close(fd_read);
                    return XST_FAILURE;
                }
                if (bytes_read == 0) break;
                total_read += bytes_read;
            }

            if (total_read != frame_bytes) {
                printf("Incomplete frame read: expected %zu, got %zd\n", frame_bytes, total_read);
                munmap(shm_ptr, shm_size);
                close(shm_fd);
                free(input_buffer);
                close(fd_read);
                return XST_FAILURE;
            }

            uint64_t* pixels_64 = (uint64_t*)input_buffer;

            for (size_t row = 0; row < FRAME_HEIGHT; row++) {
                size_t out_col = 0;
                for (size_t col = 0; col < stride_2pixels; col++) {
                    uint64_t word = pixels_64[row * stride_2pixels + col];

                    uint32_t pix1 = (uint32_t)((word) & 0x3FFFFFFF);
                    uint16_t g10_1 = pix1 & 0x3FF;
                    uint16_t b10_1 = (pix1 >> 10) & 0x3FF;
                    uint16_t r10_1 = (pix1 >> 20) & 0x3FF;

                    if (out_col < FRAME_WIDTH) {
                        //output_buffer[(row * FRAME_WIDTH + out_col) * 3 + 0] = 0;
                        //output_buffer[(row * FRAME_WIDTH + out_col) * 3 + 1] = 0;
                        //output_buffer[(row * FRAME_WIDTH + out_col) * 3 + 2] = 0;

                        output_buffer[(row * FRAME_WIDTH + out_col) * 3 + 0] = b10_1;
                        output_buffer[(row * FRAME_WIDTH + out_col) * 3 + 1] = g10_1;
                        output_buffer[(row * FRAME_WIDTH + out_col) * 3 + 2] = r10_1;

                        //output_buffer[(row * FRAME_WIDTH + out_col) * 3 + 0] = convert_10bit_to_8bit(b10_1);
                        //output_buffer[(row * FRAME_WIDTH + out_col) * 3 + 1] = convert_10bit_to_8bit(g10_1);
                        //output_buffer[(row * FRAME_WIDTH + out_col) * 3 + 2] = convert_10bit_to_8bit(r10_1);
                    }
                    out_col++;

                    uint32_t pix2 = (uint32_t)((word >> 32) & 0x3FFFFFFF);
                    uint16_t g10_2 = pix2 & 0x3FF;
                    uint16_t b10_2 = (pix2 >> 10) & 0x3FF;
                    uint16_t r10_2 = (pix2 >> 20) & 0x3FF;

                    if (out_col < FRAME_WIDTH) {
                        //output_buffer[(row * FRAME_WIDTH + out_col) * 3 + 0] = 0;
                        //output_buffer[(row * FRAME_WIDTH + out_col) * 3 + 1] = 0;
                        //output_buffer[(row * FRAME_WIDTH + out_col) * 3 + 2] = 0;

                        output_buffer[(row * FRAME_WIDTH + out_col) * 3 + 0] = b10_2;
                        output_buffer[(row * FRAME_WIDTH + out_col) * 3 + 1] = g10_2;
                        output_buffer[(row * FRAME_WIDTH + out_col) * 3 + 2] = r10_2;

                        //output_buffer[(row * FRAME_WIDTH + out_col) * 3 + 0] = convert_10bit_to_8bit(b10_2);
                        //output_buffer[(row * FRAME_WIDTH + out_col) * 3 + 1] = convert_10bit_to_8bit(g10_2);
                        //output_buffer[(row * FRAME_WIDTH + out_col) * 3 + 2] = convert_10bit_to_8bit(r10_2);
                    }
                    out_col++;
                }
            }

            (*frame_counter)++;  // Signal new frame is ready
    }

    // Cleanup
    munmap(shm_ptr, shm_size);
    close(shm_fd);
    close(fd_read);
    free(input_buffer);

    return XST_SUCCESS;
}


int streaming() {
    //int fd_read;
    ssize_t bytes_read, total_read;
    size_t padded_width = ((FRAME_WIDTH + 7) / 8) * 8;
    size_t stride_2pixels = padded_width / 2; // number of 64-bit words per line
    size_t stride_bytes = stride_2pixels * 8; // 8 bytes per 2 pixels
    size_t frame_bytes = stride_bytes * FRAME_HEIGHT;

    uint8_t* input_buffer = malloc(frame_bytes);
    if (!input_buffer) {
        printf("Malloc failed for input_buffer\n");
        return XST_FAILURE;
    }

    uint8_t* output_buffer = malloc(FRAME_WIDTH * FRAME_HEIGHT * 3);
    if (!output_buffer) {
        printf("Malloc failed for output_buffer\n");
        free(input_buffer);
        return XST_FAILURE;
    }

    fd_read = open("/dev/xdma0_c2h_0", O_RDONLY);
    if (fd_read < 0) {
        printf("Failed to open DMA device\n");
        free(input_buffer);
        free(output_buffer);
        return XST_FAILURE;
    }

    while(1)
    {

    total_read = 0;
    while (total_read < frame_bytes) {
        //bytes_read = pread(fd_read, input_buffer + total_read, frame_bytes - total_read, total_read);
        bytes_read = read(fd_read, input_buffer + total_read, frame_bytes - total_read);
        if (bytes_read < 0) {
            printf("Failed to read from DMA\n");
            free(input_buffer);
            free(output_buffer);
            close(fd_read);
            return XST_FAILURE;
        }
        if (bytes_read == 0) break; // EOF unexpected here
        total_read += bytes_read;
    }

    if (total_read != frame_bytes) {
        printf("Incomplete frame read: expected %zu, got %zd\n", frame_bytes, total_read);
        free(input_buffer);
        free(output_buffer);
        close(fd_read);
        return XST_FAILURE;
    }
    
    // I used chatgpt for this. Dont kill me
    // Need to do this is hardware instead. Too CPU intensive
    // Unpack pixels
    // Treat input_buffer as array of 64-bit words (2 pixels each)
    uint64_t* pixels_64 = (uint64_t*)input_buffer;

    for (size_t row = 0; row < FRAME_HEIGHT; row++) {
        size_t out_col = 0;
        for (size_t col = 0; col < stride_2pixels; col++) {
            uint64_t word = pixels_64[row * stride_2pixels + col];

            // Extract first pixel (bits 0-29)
            uint32_t pix1 = (uint32_t)(word & 0x3FFFFFFF);
            uint16_t g10_1 = pix1 & 0x3FF;
            uint16_t b10_1 = (pix1 >> 10) & 0x3FF;
            uint16_t r10_1 = (pix1 >> 20) & 0x3FF;
            if (out_col < FRAME_WIDTH) {
                output_buffer[(row * FRAME_WIDTH + out_col) * 3 + 0] = convert_10bit_to_8bit(g10_1);
                output_buffer[(row * FRAME_WIDTH + out_col) * 3 + 1] = convert_10bit_to_8bit(b10_1);
                output_buffer[(row * FRAME_WIDTH + out_col) * 3 + 2] = convert_10bit_to_8bit(r10_1);
            }
            out_col++;

            // Extract second pixel (bits 30-59)
            uint32_t pix2 = (uint32_t)((word >> 30) & 0x3FFFFFFF);
            uint16_t g10_2 = pix2 & 0x3FF;
            uint16_t b10_2 = (pix2 >> 10) & 0x3FF;
            uint16_t r10_2 = (pix2 >> 20) & 0x3FF;
            if (out_col < FRAME_WIDTH) {
                output_buffer[(row * FRAME_WIDTH + out_col) * 3 + 0] = convert_10bit_to_8bit(g10_2);
                output_buffer[(row * FRAME_WIDTH + out_col) * 3 + 1] = convert_10bit_to_8bit(b10_2);
                output_buffer[(row * FRAME_WIDTH + out_col) * 3 + 2] = convert_10bit_to_8bit(r10_2);
            }
            out_col++;
        }
    }
    

    FILE* fout = fopen("frame.rgb", "a");
    if (!fout) {
        printf("Failed to open output file\n");
        free(input_buffer);
        free(output_buffer);
        close(fd_read);
        return XST_FAILURE;
    }

    fwrite(input_buffer, 1, FRAME_WIDTH * FRAME_HEIGHT * 3, fout);
    fclose(fout);

    }

    free(input_buffer);
    free(output_buffer);
    close(fd_read);

    return XST_SUCCESS;
}


int streaming3() {
    ssize_t bytes_read, total_read;
    size_t padded_width = ((FRAME_WIDTH + 7) / 8) * 8; // Ensure width aligns to 8
    size_t stride_2pixels = padded_width / 2;
    size_t num_256bit_per_row = (stride_2pixels + 3) / 4; // Adjust for 256-bit chunks
    size_t stride_bytes = num_256bit_per_row * 32; // 32 bytes per 256-bit chunk
    size_t frame_bytes = stride_bytes * FRAME_HEIGHT;

    // Allocate input buffer with DMA alignment
    uint8_t *input_buffer;
    if (posix_memalign((void **)&input_buffer, DMA_ALIGNMENT, frame_bytes) != 0) {
        printf("Failed to allocate aligned input_buffer\n");
        return EXIT_FAILURE;
    }

    // Set up shared memory
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd < 0) {
        perror("shm_open");
        free(input_buffer);
        return EXIT_FAILURE;
    }

    // Set shared memory size (frame buffer + control integer)
    size_t shm_size = (sizeof(uint32_t) + FRAME_SIZE);
    //size_t shm_size = FRAME_SIZE;
    if (ftruncate(shm_fd, shm_size) == -1) {
        perror("ftruncate");
        close(shm_fd);
        free(input_buffer);
        return EXIT_FAILURE;
    }

    uint8_t *shm_ptr = mmap(NULL, shm_size, PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        perror("mmap");
        close(shm_fd);
        free(input_buffer);
        return EXIT_FAILURE;
    }

    uint32_t *frame_counter = (uint32_t *)shm_ptr;
    uint8_t *output_buffer = shm_ptr + sizeof(uint32_t);
    //uint8_t *output_buffer = shm_ptr;

    // Open DMA device
    int fd_read = open("/dev/xdma0_c2h_0", O_RDONLY);
    if (fd_read < 0) {
        printf("Failed to open DMA device\n");
        munmap(shm_ptr, shm_size);
        close(shm_fd);
        free(input_buffer);
        return EXIT_FAILURE;
    }

    // Start streaming loop
    while (1) {
        total_read = 0;
        while (total_read < frame_bytes) {
            bytes_read = read(fd_read, input_buffer + total_read, frame_bytes - total_read);
            if (bytes_read < 0) {
                printf("Failed to read from DMA\n");
                munmap(shm_ptr, shm_size);
                close(shm_fd);
                free(input_buffer);
                close(fd_read);
                return EXIT_FAILURE;
            }
            if (bytes_read == 0) break;
            total_read += bytes_read;
        }

        if (total_read != frame_bytes) {
            printf("Incomplete frame read: expected %zu, got %zd\n", frame_bytes, total_read);
            munmap(shm_ptr, shm_size);
            close(shm_fd);
            free(input_buffer);
            close(fd_read);
            return EXIT_FAILURE;
        }

        uint64_t *pixels_256 = (uint64_t *)input_buffer;

        // Process and store pixels correctly
        for (size_t row = 0; row < FRAME_HEIGHT; row++) {
            size_t out_col = 0;
            for (size_t col = 0; col < num_256bit_per_row; col++) {
                uint64_t word1 = pixels_256[row * num_256bit_per_row + col * 4 + 0];
                uint64_t word2 = pixels_256[row * num_256bit_per_row + col * 4 + 1];
                uint64_t word3 = pixels_256[row * num_256bit_per_row + col * 4 + 2];
                uint64_t word4 = pixels_256[row * num_256bit_per_row + col * 4 + 3];

                uint64_t words[] = {word1, word2, word3, word4};

                for (int i = 0; i < 4; i++) {
                    uint32_t pix1 = (uint32_t)((words[i]) & (0x3FFFFFFF));
                    uint32_t pix2 = (uint32_t)((words[i] >> 30) & (0x3FFFFFFF));

                    uint16_t r10_1 = (pix1 >> 20) & 0x3FF;
                    uint16_t b10_1 = (pix1 >> 10) & 0x3FF;
                    uint16_t g10_1 = pix1 & 0x3FF;

                    uint16_t r10_2 = (pix2 >> 20) & 0x3FF;
                    uint16_t b10_2 = (pix2 >> 10) & 0x3FF;
                    uint16_t g10_2 = pix2 & 0x3FF;

                    if (out_col < FRAME_WIDTH) {
                        /*
                        output_buffer[(row * FRAME_WIDTH + out_col) * 3 + 0] = b10_1;
                        output_buffer[(row * FRAME_WIDTH + out_col) * 3 + 1] = g10_1;
                        output_buffer[(row * FRAME_WIDTH + out_col) * 3 + 2] = r10_1;
                        */
                        
                        output_buffer[(row * FRAME_WIDTH + out_col) * 3 + 0] = convert_10bit_to_8bit(b10_1);
                        output_buffer[(row * FRAME_WIDTH + out_col) * 3 + 1] = convert_10bit_to_8bit(g10_1);
                        output_buffer[(row * FRAME_WIDTH + out_col) * 3 + 2] = convert_10bit_to_8bit(r10_1);
                        
                    }
                    out_col++;

                    if (out_col < FRAME_WIDTH) {
                        /*
                        output_buffer[(row * FRAME_WIDTH + out_col) * 3 + 0] = b10_2;
                        output_buffer[(row * FRAME_WIDTH + out_col) * 3 + 1] = g10_2;
                        output_buffer[(row * FRAME_WIDTH + out_col) * 3 + 2] = r10_2;
                        */
                        
                        output_buffer[(row * FRAME_WIDTH + out_col) * 3 + 0] = convert_10bit_to_8bit(b10_2);
                        output_buffer[(row * FRAME_WIDTH + out_col) * 3 + 1] = convert_10bit_to_8bit(g10_2);
                        output_buffer[(row * FRAME_WIDTH + out_col) * 3 + 2] = convert_10bit_to_8bit(r10_2);
                        
                    }
                    out_col++;
                }

            }

        }
        (*frame_counter)++; // Signal new frame is ready
    }

    // Cleanup
    munmap(shm_ptr, shm_size);
    close(shm_fd);
    close(fd_read);
    free(input_buffer);

    return EXIT_SUCCESS;
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

    status = XGpio_Initialize(&tpgresetInst, XPAR_AXI_TPG_RESET_BASEADDR);
    if(status != XST_SUCCESS)
    {
        printf("Failed to Init TPG RESET GPIO! \r\n");
        return XST_FAILURE;
    }

    XGpio_DiscreteWrite(&tpgresetInst, 0x01, 0x01);

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

    tpg();

    printf("Doing DMA Stuff! \r\n");

    status = streaming2();
    if(status != XST_SUCCESS)
    {
        printf("Failed to Capture Frame! \r\n");
        return XST_FAILURE;
    }

    uint32_t read_buff[2];
    uint32_t write_buff[2];

    int x = 0;
    while(x <= 1)
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
            x++;
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

    XV_tpg_DisableAutoRestart(&tpgInst);
    tpg_reset();

    //remove("frame.rgb");

    close(fd);
    close(fd_read);

    return XST_SUCCESS;
}

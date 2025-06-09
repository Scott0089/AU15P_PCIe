#include "sc_io.h"

uint32_t SC_In32(uint64_t addr)
{
    uint32_t data_word = 0x0;
    //ssize_t rc = pread(fd, &data_word, 4, addr-TARGET_PHYS_ADDR);
    pread(fd, &data_word, 4, addr-TARGET_PHYS_ADDR);
    return data_word;

}

void SC_Out32(uint64_t addr, uint32_t data)
{
    //ssize_t rc = pwrite(fd, &data, 4, addr-TARGET_PHYS_ADDR);

    pwrite(fd, &data, 4, addr-TARGET_PHYS_ADDR);
}

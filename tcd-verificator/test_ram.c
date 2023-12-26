
#include "main.h"

u8 tst_bram2k() {

    u16 i;
    u16 *ram = (u16 *) ADDR_RAM_BANK;

    sysSetBank(TCD_BANK_BRAM);

    //turn on bram and write some values
    CD_REG_1807 = 0x80;

    ram[0] = 0xaaaa;
    ram[1] = 0x5555;
    ram[2] = 0x1234;

    if (ram[0] != 0xaaaa)return 0x01;
    if (ram[1] != 0x5555)return 0x01;
    if (ram[2] != 0x1234)return 0x01;

    //turn off bram. read soudn't work also
    CD_REG_1807 = 0x00;
    if (ram[0] == 0xaaaa)return 0x02;

    //try to modify ram while it turned off
    ram[0] == 0;
    CD_REG_1807 = 0x80;
    if (ram[0] != 0xaaaa)return 0x03;

    //read status should turn off bram
    i = CD_REG_1803;
    if (ram[0] == 0xaaaa)return 0x04;

    //try to modify ram while it turned off by status reading
    ram[0] == 0;
    CD_REG_1807 = 0x80;
    if (ram[0] != 0xaaaa)return 0x05;

    //fill whole ram and try read all back
    CD_REG_1807 = 0x80;
    for (i = 0; i < 2048 / 2; i++) {
        ram[i] = i;
    }

    for (i = 0; i < 2048 / 2; i++) {
        if (ram[i] != i)return 0x06;
    }

    //now with all values inverted
    for (i = 0; i < 2048 / 2; i++) {
        ram[i] ^= 0xffff;
    }

    for (i = 0; i < 2048 / 2; i++) {
        if (ram[i] != (i ^ 0xffff))return 0x07;
    }

    //make sure bram is not mirrored
    ram[1024 * 0] = 0xaa55;
    ram[1024 * 1] = 0;
    ram[1024 * 2] = 0;
    ram[1024 * 3] = 0;
    if (ram[1024 * 0] != 0xaa55)return 0x08;
    if (ram[1024 * 1] == 0xaa55)return 0x08;
    if (ram[1024 * 2] == 0xaa55)return 0x08;
    if (ram[1024 * 3] == 0xaa55)return 0x08;


    return 0;
}

u8 tst_wram() {

    u16 i;
    u16 *ram16 = (u16 *) ADDR_RAM_BANK;
    u8 *ram8 = (u8 *) ADDR_RAM_BANK;

    sysSetBank(TCD_BANK_WRAM);

    //basi test. write few bytes
    ram16[0] = 0xaaaa;
    ram16[1] = 0x5555;
    ram16[2] = 0x1234;

    if (ram16[0] != 0xaaaa)return 0x01;
    if (ram16[1] != 0x5555)return 0x01;
    if (ram16[2] != 0x1234)return 0x01;

    //test first 256 byte
    for (i = 0; i < 256; i++) {
        ram8[i] = i;
    }

    for (i = 0; i < 256; i++) {
        if (ram8[i] != i)return 0x02;
    }

    //test 8K bank
    for (i = 0; i < 8192; i += 256) {
        ram8[i + 0] = i / 256;
        ram8[i + 1] = i / 256 ^ 0xff;
    }

    for (i = 0; i < 8192; i += 256) {
        if (ram8[i + 0] != i / 256)return 0x03;
        if (ram8[i + 1] != (i / 256 ^ 0xff))return 0x03;
    }

    //test 8x8K banks
    for (i = 0; i < 8; i++) {
        sysSetBank(TCD_BANK_WRAM + i);
        ram8[0] = i;
        ram8[1] = i ^ 0xff;
    }

    for (i = 0; i < 8; i++) {
        sysSetBank(TCD_BANK_WRAM + i);
        if (ram8[0] != i)return 0x04;
        if (ram8[1] != (i ^ 0xff))return 0x04;
    }

    return 0;
}

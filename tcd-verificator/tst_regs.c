
#include "main.h"

u8 tst_regs_rw() {

    u16 i;
    u8 val[2];

    cd_reg_w(1804, 0xff);
    gVsync();
    val[0] = cd_reg_r(1804);
    gVsync();
    gVsync();
    cd_reg_w(1804, 0x00);
    gVsync();
    val[1] = cd_reg_r(1804);
    gVsync();

    //gConsPrint("vals:");
    //gAppendHex8(val[0]);
    //gAppendHex8(val[1]);

    //not sure about this
    if (val[0] != 0xff)return 1;
    if (val[1] != 0x00)return 1;

    for (i = 0; i < 16; i++) {
        cd_reg_w(0x1801, 0x2a);
        if (cd_reg_r(0x1801) == 0x2a)break;
        gVsync();
    }

    //val in 1801 should be mirrored to 1808
    cd_reg_w(0x1801, 0xaa);
    cd_reg_w(0x1801, 0xaa);
    if (cd_reg_r(0x1808) != 0xaa)return 2;
    cd_reg_w(0x1801, 0x55);
    if (cd_reg_r(0x1808) != 0x55)return 2;

    //1808 is read only
    cd_reg_w(0x1808, 0xaa);
    if (cd_reg_r(0x1801) != 0x55)return 3;


    //all bits r/w
    cd_reg_w(0x1802, 0xff);
    if (cd_reg_r(0x1802) != 0xff)return 4;
    cd_reg_w(0x1802, 0x00);
    if (cd_reg_r(0x1802) != 0x00)return 4;

    cd_reg_w(0x180d, 0xff);
    if (cd_reg_r(0x180d) != 0xff)return 5;
    cd_reg_w(0x180d, 0x00);
    if (cd_reg_r(0x180d) != 0x00)return 5;

    cd_reg_w(0x180e, 0xff);
    if (cd_reg_r(0x180e) != 0xff)return 6;
    cd_reg_w(0x180e, 0x00);
    if (cd_reg_r(0x180e) != 0x00)return 6;

    cd_reg_w(0x180f, 0xff);
    if (cd_reg_r(0x180f) != 0xff)return 7;
    cd_reg_w(0x180f, 0x00);
    if (cd_reg_r(0x180f) != 0x00)return 7;


    cd_reg_w(0x180d, 0x80);
    gVsync();
    if ((cd_reg_r(0x180c) & 1) != 0x00)return 8;
    cd_reg_w(0x180d, 0x00);


    //registers mirroring check. should be mapped only in first 16 bytes
    cd_reg_w(0x180f, 0x00);
    for (i = 16; i < 1024; i += 16) {
        cd_reg_w(0x180f + i, 0xff);
        if (cd_reg_r(0x180f) != 0x00)return 9;
    }

    return 0;
}

u8 tst_regs_1803_1() {

    u8 i;
    u8 v0 = 0;
    u8 v1 = 0;

    //check if chan togglw works at all
    i = 0;
    while ((CD_REG_1803 & 2) != 0) {
        CD_REG_1805 = 0;
        if (i++ >= 128)return 1;
    }
    i = 0;
    while ((CD_REG_1803 & 2) == 0) {
        CD_REG_1805 = 0;
        if (i++ >= 128)return 1;
    }

    //sync
    CD_REG_1805 = 0;
    while ((CD_REG_1803 & 2) == 0) {
        CD_REG_1805 = 0;
    }
    CD_REG_1805 = 0;
    while ((CD_REG_1803 & 2) != 0) {
        CD_REG_1805 = 0;
    }

    //check time
    for (i = 0; i < 64;) {
        CD_REG_1805 = 0;
        cbuff[i++] = CD_REG_1803;
    }


    for (i = 0; i < 64; i++) {
        cbuff[i] &= 2;
    }

    //sync. first results may be not relevant
    i = 0;
    while (i < 64 && cbuff[i] == 0) {
        i++;
    }
    while (i < 64 && cbuff[i] != 0) {
        i++;
    }

    while (i < 64 && cbuff[i] == 0) {
        v0++;
        i++;
    }

    while (i < 64 && cbuff[i] != 0) {
        v1++;
        i++;
    }

    //normal result is 5
    if (v0 > 5 + 1)return 2;
    if (v0 < 5 - 1)return 3;
    if (v1 > 5 + 1)return 4;
    if (v1 < 5 - 1)return 5;


    //1806 shouldn't affect
    v0 = CD_REG_1803 & 2;
    asm("nop");
    v0 = CD_REG_1803 & 2;
    for (i = 0; i < 128; i++) {
        CD_REG_1806 = 0;
        asm("nop");
        if ((CD_REG_1803 & 2) != v0)return 6;
        asm("nop");
    }

    return 0;
}

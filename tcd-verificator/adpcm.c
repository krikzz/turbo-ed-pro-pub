
#include "main.h"

void ad_set_rd();
void ad_set_wr();
void ad_set_len();
void ad_rst();

u8 ad_rate;

void ad_reset() {

    //return;
    ad_rate = 12; //8khz
    ad_rst();
    CD_REG_180F = 0x00; //set byte mode for adpcm ram

}

void ad_set_addr_rd(u16 addr) {

    CD_REG_1808_16 = addr;
    ad_set_rd();
    /*
    static vu8 tmp;
    while ((CD_REG_180C & 0x08));
    CD_REG_1808_16 = addr;
    tmp = CD_REG_180D;
    CD_REG_180D = 0x0c; //4 swap nibbles, 8 do nothing, c set addr
    tmp = CD_REG_180D;
    CD_REG_180D = 0x00;*/
}

void ad_set_addr_wr(u16 addr) {

    CD_REG_1808_16 = addr;
    ad_set_wr();
    /*
    //work of 0x03/0x0c mask unclear. in some cases it doesn work. 
    //0x0f seems works always
    static vu8 tmp;
    while ((CD_REG_180C & 0x08));

    CD_REG_1808_16 = addr;
    tmp = CD_REG_180D;
    CD_REG_180D = 0x03; //4 swap nibbles, 8 do nothing, c set addr
    tmp = CD_REG_180D;
    CD_REG_180D = 0x00;*/
}

void ad_play_busy() {

    while ((CD_REG_180C & 1) == 0);
}

u8 ad_byte_rd() {

    while ((CD_REG_180C & 0x08));
    return CD_REG_180A;
}


void ad_byte_wr(u8 val) {

    while ((CD_REG_180C & 0x08));
    CD_REG_180A = val;
}

void ad_play_start(u16 addr, u16 size, u8 loop) {

    ad_set_addr_rd(addr);

    //set sample len
    CD_REG_1808_16 = size;
    CD_REG_180D = 0x10;
    CD_REG_180D = 0x00;

    CD_REG_180E = ad_rate; //data rate
    //CD_REG_1802 = 0x08; //half sample elapsed flag
    if (loop) {
        CD_REG_180D = 0x20; // play and stop in the end (bit 6 is stop option)
    } else {
        CD_REG_180D = 0x60; // play and stop in the end (bit 6 is stop option)
    }

    ad_rate = 12; //set default 8khz rate
}

void ad_set_rate(u8 rate) {
    ad_rate = rate;
}

void ad_play_stop() {
    CD_REG_180D = 0x00;
}

void ad_ram_rd(u8 *dst, u16 addr, u16 len) {

    ad_set_addr_rd(addr);
    ad_byte_rd(); //push first byte to the buffer

    while (len--) {
        *dst++ = ad_byte_rd();
    }
}

void ad_ram_wr(u8 *src, u16 addr, u16 len) {

    ad_set_addr_wr(addr);

    while (len--) {
        ad_byte_wr(*src++);
    }
}

void ad_dma_start(u16 dst) {

    //CD_REG_1808_16 = 1; //0x8001;
    //CD_REG_180D = 0x10; //clear 180C bit 0 (set sample len)
    //CD_REG_180D = 0x00;

    ad_set_addr_wr(dst);

    CD_REG_180B = 0x02; //run dma

}

void ad_set_sample_len(u16 len) {

    CD_REG_1808_16 = len; //0x8001;
    CD_REG_180D = 0x10; //clear 180C bit 0 (set sample len)
    CD_REG_180D = 0x00;
}

void ad_dma_busy() {

    //bios doing this way
    while (1) {
        if ((CD_REG_180C & 0x02) != 0)continue;
        if ((CD_REG_1803 & 0x20) == 0)continue;
        break;
    }
}

u8 ad_play_end() {
    return (CD_REG_1803 & 0x08);
}

u8 ad_play_end_32K() {
    return (CD_REG_1803 & 0x04);
}
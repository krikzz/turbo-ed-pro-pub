
#include "main.h"

u8 tst_ad_play() {

    u32 i, u;

    ad_reset();

    //load sample
    for (i = 0; i < 0xc000; i += sizeof (sample)) {
        ad_ram_wr(sample, i, sizeof (sample));
    }

    //check if playback flags work at all and their timings
    gVsync();
    ad_play_start(0, 1024, 0);

    i = 0;
    while (!ad_play_end()) {
        i++;
        gVsync();
        if (i >= 16 + 32)break;
    }

    if (i >= 16 + 32)return 0x01; //seems flag does not work at all
    if (i >= 16 + 2)return 0x02; //fire to late
    if (i <= 16 - 2)return 0x03; //fire to fast


    //repeat test, but with 4Khz rate. timing should be doubled
    ad_set_rate(8);
    gVsync();
    ad_play_start(0, 1024, 0);
    i = 0;
    while (!ad_play_end()) {
        i++;
        gVsync();
    }

    if (i >= 31 + 2)return 0x04; //fire to late
    if (i <= 31 - 2)return 0x05; //fire to fast

    //end_32K flag always should be set if ramain sample less than 32kb
    ad_play_start(0, 1024, 0);
    gVsync();
    if (!ad_play_end_32K())return 0x06;
    while (!ad_play_end());


    //check end_32K timings
    ad_set_rate(15); //32Khz
    gVsync();
    ad_play_start(0, 36864L, 0); //32+4K
    gVsync();

    i = 0;
    u = 0;
    while (!ad_play_end()) {
        i++;
        if (!ad_play_end_32K())u++;
        gVsync();
    }

    if (i >= 137 + 2)return 0x07; //fire to late
    if (i <= 137 - 2)return 0x08; //fire to fast
    if (u >= 15 + 2)return 0x09; //fire to late
    if (u <= 15 - 2)return 0x0A; //fire to fast

    //****************************************************
    //playback should change read address
    for (i = 0; i < 128; i++) {
        cbuff[i + 0] = 0x11; //i;
        cbuff[i + 128] = 0x22; //i;
        cbuff[i + 256] = 0x33;
    }

    ad_ram_wr(cbuff, 0, 512);
    ad_set_addr_wr(0);

    ad_play_start(0, 128 - 2, 0);

    ad_play_busy();
    ad_byte_rd();

    for (i = 0; i < 256; i++) {
        cbuff[i] = ad_byte_rd();
    }

    if (cbuff[0] != 0x11)return 0x10;
    if (cbuff[2] != 0x22)return 0x11;
    if (cbuff[128] != 0x22)return 0x12;
    if (cbuff[130] != 0x33)return 0x13;

    //wr addr should stay unchanged
    ad_byte_wr(0x12);
    ad_byte_wr(0x34);
    ad_set_addr_rd(0);
    ad_byte_rd();
    if (ad_byte_rd() != 0x12)return 0x14;
    if (ad_byte_rd() != 0x34)return 0x15;

    //gPrintHex(cbuff, 256);
    //****************************************************


    return 0;
}

u8 tst_ad_irq(u8 init) {

    u32 i;

    ad_reset();

    if (init == 0) {
        sysIrqON();
        return tst_ad_irq(1);
        sysIrqOFF();
    }

    //check if irq work at all and it timings
    gVsync();
    ad_play_start(0, 1024, 0);
    gVsync();
    irq_ctr = 0;
    CD_REG_1802 = 0x08; //enable irq for the end of playback

    i = 0;
    while (irq_ctr == 0) {
        i++;
        gVsync();
        if (i >= 16 + 32)break;
    }

    if (i >= 15 + 32)return 0x01; //seems flag does not work at all
    if (i >= 15 + 2)return 0x02; //fire to late
    if (i <= 15 - 2)return 0x03; //fire to fast


    //check if end_32K irq work and it timings
    gVsync();
    ad_play_start(0, 33792L, 0); //32K+1K
    gVsync();
    irq_ctr = 0;
    CD_REG_1802 = 0x04; //enable irq for the less than 32K sample remains

    i = 0;
    while (irq_ctr == 0) {
        i++;
        gVsync();
        if (i >= 16 + 32)break;
    }

    ad_play_stop();

    if (i >= 15 + 32)return 0x04; //seems flag does not work at all
    if (i >= 15 + 2)return 0x05; //fire to late
    if (i <= 15 - 2)return 0x06; //fire to fast


    //irq should fire just once. but if enable it again it should fire immediately
    gVsync();
    ad_play_start(0, 1024, 0);
    gVsync();
    irq_ctr = 0;
    CD_REG_1802 = 0x08; //enable irq for the end of playback

    while (irq_ctr == 0) {
        gVsync();
    }
    //if irq generated tpen playbac end flag also should be set
    if (!ad_play_end())return 0x07;

    //should fire just once
    if (irq_ctr != 1)return 0x08;

    //should fire immediately if enable irq while ad_play_end state
    CD_REG_1802 = 0x08;
    asm("nop");
    asm("nop");
    asm("nop");
    if (irq_ctr != 2)return 0x09;


    //move len counter using read/write via 180A
    CD_REG_180D = 0; //stop playback
    irq_ctr = 0;
    ad_set_sample_len(1);
    CD_REG_1802 = 0x08; //enable irq for the end of playback

    gVsync();
    if (irq_ctr != 0)return 0x10; //lctr=1, eflag = 0

    ad_byte_rd();
    gVsync();
    if (irq_ctr != 0)return 0x11; //lctr=0, eflag = 0

    ad_byte_rd();
    gVsync();
    if (irq_ctr != 1)return 0x12; //lctr=0, eflag = 1

    CD_REG_1802 = 0x04; //enable irq 32k flag

    gVsync();
    if (irq_ctr != 1)return 0x13; //lctr=0, 32kflag = 0

    ad_byte_wr(0);
    gVsync();
    if (irq_ctr != 2)return 0x14; //lctr=1, 32kflag = 1


    return 0;
}

u8 tst_ad_ram_mode() {

    u16 i;

    for (i = 0; i < 16; i++) {
        cbuff[i] = i;
    }

    //test byte mode
    CD_REG_180F = 0x00; //regular byte mode
    ad_ram_wr(cbuff, 0, 16);
    ad_ram_rd(cbuff, 0, 16);

    for (i = 0; i < 16; i++) {
        if (cbuff[i] != i)return 1;
    }

    //test block mode
    CD_REG_180F = 0x80; //8byte block mode
    ad_ram_wr(cbuff, 0, 8);
    ad_ram_wr(cbuff + 8, 1, 8);
    ad_ram_rd(cbuff, 0, 8);
    ad_ram_rd(cbuff + 8, 1, 8);
    CD_REG_180F = 0x00; //8byte block mode

    for (i = 0; i < 16; i++) {
        if (cbuff[i] != i)return 2;
    }

    //test wrapping in block mode. only block 0 should be modified
    for (i = 0; i < 16; i++) {
        cbuff[i] = 0x10 + i;
    }
    CD_REG_180F = 0x80; //8byte block mode
    ad_ram_wr(cbuff, 0, 16);
    ad_ram_rd(cbuff, 0, 16);
    CD_REG_180F = 0x00; //8byte block mode

    for (i = 0; i < 16; i++) {
        if (cbuff[i] != i % 8 + 0x18)return 3;
    }

    //check data in block 1 (should be unchanged)
    CD_REG_180F = 0x80; //8byte block mode
    ad_ram_rd(cbuff, 1, 8);
    CD_REG_180F = 0x00; //8byte block mode

    for (i = 0; i < 8; i++) {
        if (cbuff[i] != i + 8)return 4;
    }

    //addressing in block mode: 
    //0X = addr * 8738          stored in first 4 bits
    //X0 = addr * 8738 + 4369   stored last 4 bits
    //stored in

    return 0;
}

u8 tst_ad_len_ctr() {

    u16 resp;
    u16 i;

    /*
    CD_REG_180E = 0; //rate
    ad_set_sample_len(8);
    while ((CD_REG_180C & 1) == 0) {
        resp = CD_REG_180A;
    }*/

    ad_reset();
    gVsync();

    CD_REG_180B = 0; //turn off dma
    CD_REG_180D = 0x00;
    CD_REG_180E = 0; //rate
    //test flags related to sample len
    gVsync();

    //default values after reset
    if ((rd_180C() & 0x01) != 0x00)return 0x01;
    if ((rd_1803() & 0x0c) != 0x00)return 0x02;

    //should set sample end flag and 180C:0(1803:3 mirror?)
    ad_set_sample_len(0x0000);
    resp = CD_REG_180A;
    if ((rd_180C() & 0x01) != 0x01)return 0x03;
    if ((rd_1803() & 0x0c) != 0x08)return 0x04;

    //should set "32k remains" flag
    ad_set_sample_len(0x0001);
    resp = CD_REG_180A;
    if ((rd_180C() & 0x01) != 0x00)return 0x05;
    if ((rd_1803() & 0x0c) != 0x04)return 0x06;

    //still should be set
    ad_set_sample_len(0x8000);
    resp = CD_REG_180A;
    if ((rd_180C() & 0x01) != 0x00)return 0x07;
    if ((rd_1803() & 0x0c) != 0x04)return 0x08;

    //all flags should be cleared
    ad_set_sample_len(0x8001);
    resp = CD_REG_180A;
    if ((rd_180C() & 0x01) != 0x00)return 0x09;
    if ((rd_1803() & 0x0c) != 0x00)return 0x10;


    //after play end status always should be 01/0c
    ad_set_sample_len(128);
    CD_REG_180E = 15; //max rate
    CD_REG_180D = 0x60; //play and stop
    gVsync();
    gVsync();
    gVsync();
    gVsync();
    gVsync();
    gVsync();
    /*
    gConsPrint("val: ");
    gAppendHex8(rd_180C());
    gConsPrint("val: ");
    gAppendHex8(rd_1803());*/
    //resp = CD_REG_180A;
    //resp = CD_REG_180A;
    if ((rd_180C() & 0x01) != 0x01)return 0x11;
    if ((rd_1803() & 0x0c) != 0x0c)return 0x12;

    //now test len counter manipulations using 180A. 
    //every read should clock the counter
    ad_set_sample_len(2);
    resp = CD_REG_180A; //count from 2 to 1
    if ((rd_1803() & 0x0c) != 0x04)return 0x13;
    resp = CD_REG_180A; //count from 2 to 0
    if ((rd_1803() & 0x0c) != 0x04)return 0x14;
    resp = CD_REG_180A; //count from 0 to ?
    if ((rd_1803() & 0x0c) != 0x08)return 0x15;

    ad_set_sample_len(0x8002);
    resp = CD_REG_180A;
    if ((rd_1803() & 0x0c) != 0x00)return 0x16;
    resp = CD_REG_180A;
    if ((rd_1803() & 0x0c) != 0x00)return 0x17;
    resp = CD_REG_180A;
    if ((rd_1803() & 0x0c) != 0x04)return 0x18;

    for (i = 0; i < 32767; i++) {
        resp = CD_REG_180A;
    }
    if ((rd_1803() & 0x0c) != 0x04)return 0x19;
    resp = CD_REG_180A; //count from 0 to ? 
    if ((rd_1803() & 0x0c) != 0x08)return 0x20;

    //while 180D:4 is set, 1808 constantly will be copied to sample len
    //read write does not change the counter in such conditions
    //1803:3 always 0 

    CD_REG_180D = 0x10; //clear 180C bit 0 (set sample len)

    CD_REG_1808_16 = 0x0000;
    ad_byte_wr(0);
    if ((rd_1803() & 0x0c) != 0x04)return 0x21;
    ad_byte_rd();
    if ((rd_1803() & 0x0c) != 0x04)return 0x22;

    CD_REG_1808_16 = 0x0001;
    ad_byte_wr(0);
    if ((rd_1803() & 0x0c) != 0x04)return 0x23;
    ad_byte_rd();
    if ((rd_1803() & 0x0c) != 0x04)return 0x24;

    CD_REG_1808_16 = 0x7fff;
    ad_byte_wr(0);
    if ((rd_1803() & 0x0c) != 0x04)return 0x25;
    ad_byte_rd();
    if ((rd_1803() & 0x0c) != 0x04)return 0x26;

    CD_REG_1808_16 = 0x8000;
    ad_byte_wr(0);
    if ((rd_1803() & 0x0c) != 0x00)return 0x27;
    ad_byte_rd();
    if ((rd_1803() & 0x0c) != 0x00)return 0x28;

    CD_REG_1808_16 = 0x8001;
    ad_byte_wr(0);
    if ((rd_1803() & 0x0c) != 0x00)return 0x29;
    ad_byte_rd();
    if ((rd_1803() & 0x0c) != 0x00)return 0x2A;

    //read  180A: 1803 flags set to condition after len dec
    //write 180A: 1803 flags set to condition before len inc
    ad_set_sample_len(0x8001);
    ad_byte_rd();
    if ((rd_1803() & 0x0c) != 0x00)return 0x30;
    ad_byte_rd();
    if ((rd_1803() & 0x0c) != 0x04)return 0x31;

    ad_set_sample_len(0x7fff);
    ad_byte_wr(0);
    if ((rd_1803() & 0x0c) != 0x04)return 0x32;
    ad_byte_wr(0);
    if ((rd_1803() & 0x0c) != 0x00)return 0x33;


    //try to count up/down
    ad_set_sample_len(0x8000);

    ad_byte_rd();
    if ((rd_1803() & 0x0c) != 0x04)return 0x34; //-1

    ad_byte_wr(0);
    if ((rd_1803() & 0x0c) != 0x04)return 0x35; //0

    ad_byte_wr(0);
    if ((rd_1803() & 0x0c) != 0x00)return 0x36; //+1

    ad_byte_rd();
    if ((rd_1803() & 0x0c) != 0x00)return 0x37; //0

    ad_byte_rd();
    if ((rd_1803() & 0x0c) != 0x04)return 0x38; //-1


    //end flag sets if len counter changed when it 0
    //if dec from 0 to 0 32k flag resets
    //if inc from 0 to 1 32k flag sets
    //dec does not work if end flag set
    //len set should reset end flag, but not 32k flag
    ad_set_sample_len(1);

    ad_byte_rd();
    if ((rd_1803() & 0x0c) != 0x04)return 0x40; //4

    ad_byte_rd();
    if ((rd_1803() & 0x0c) != 0x08)return 0x41; //8

    ad_byte_wr(0);
    if ((rd_1803() & 0x0c) != 0x0c)return 0x42; //c

    ad_byte_rd();
    if ((rd_1803() & 0x0c) != 0x0c)return 0x43; //c (dec blocked)

    ad_set_sample_len(0x0000);
    if ((rd_1803() & 0x0c) != 0x04)return 0x44; //4

    ad_set_sample_len(0x8001);
    if ((rd_1803() & 0x0c) != 0x04)return 0x45; //4

    ad_byte_rd();
    if ((rd_1803() & 0x0c) != 0x00)return 0x46; //0

    ad_byte_rd();
    if ((rd_1803() & 0x0c) != 0x04)return 0x47; //4


    //writes to 180A increment len counter
    //unlike 180A read, counter not stops at ffff or 0000.
    //counter wraps, mask is 1FFFF
    //seems if counter will be leave at 0x1xxxx range it may broke proper flags work
    ad_set_sample_len(0);

    //both flags set
    ad_byte_wr(0);
    if ((CD_REG_1803 & 0x0c) != 0x0c)return 0x70;

    //count to 32k
    for (i = 0; i < 32768L - 2; i++) {
        CD_REG_180A = resp;
    }

    //0x7fff
    ad_byte_wr(0);
    if ((CD_REG_1803 & 0x0c) != 0x0c)return 0x71;

    //0x8000
    ad_byte_wr(0);
    if ((CD_REG_1803 & 0x0c) != 0x08)return 0x72;

    //go to 0x10000
    for (i = 0; i < 32768L; i++) {
        CD_REG_180A = resp;
    }
    //counter should wrap at 0x20000 range, not 0x10000
    ad_byte_wr(0);
    if ((CD_REG_1803 & 0x0c) != 0x08)return 0x73;

    //go to 0x1ffff
    for (i = 0; i < 32768L - 2; i++) {
        ad_byte_wr(0);
        ad_byte_wr(0);
    }
    ad_byte_wr(0);

    //one step before wrap
    ad_byte_wr(0);
    if ((CD_REG_1803 & 0x0c) != 0x08)return 0x74;

    //wrap point
    ad_byte_wr(0);
    if ((CD_REG_1803 & 0x0c) != 0x0c)return 0x75;



    //another wrap test
    ad_set_sample_len(0xffff);
    for (i = 0; i < 32768L; i++) {
        ad_byte_wr(0);
        ad_byte_wr(0);
    }

    if ((CD_REG_1803 & 0x0c) != 0x00)return 0x80;
    ad_byte_wr(0);
    if ((CD_REG_1803 & 0x0c) != 0x00)return 0x81;
    ad_byte_wr(0); //wrap point
    if ((CD_REG_1803 & 0x0c) != 0x0C)return 0x82;

    ad_set_sample_len(0xffff);
    ad_byte_wr(0);
    ad_byte_wr(0);
    if ((CD_REG_1803 & 0x0c) != 0x00)return 0x83;
    ad_byte_rd(0);
    ad_byte_rd(0);
    ad_byte_rd(0);
    if ((CD_REG_1803 & 0x0c) != 0x00)return 0x84;

    return 0;
}

u8 tst_ad_reset() {

    u8 resp;
    u16 i;

    //reset effects:
    //clear send and 32k flags
    //set sample len to 0
    //set rd addr to 0
    //set wr addr to 0

    CD_REG_180D = 0;
    gVsync();

    CD_REG_180D = 0x80;
    asm("nop");
    CD_REG_180D = 0x00;

    //default values after reset
    if ((rd_180C() & 0x1) != 0x00)return 0x01;
    if ((rd_1803() & 0xc) != 0x00)return 0x02;

    //set sample end bits
    ad_set_sample_len(0x0000);
    resp = CD_REG_180A;
    if ((rd_180C() & 0x1) != 0x01)return 0x03;
    if ((rd_1803() & 0xc) != 0x08)return 0x04;

    CD_REG_180D = 0x80;
    asm("nop");
    CD_REG_180D = 0x00;

    //default values after reset
    if ((rd_180C() & 0x1) != 0x00)return 0x05;
    if ((rd_1803() & 0xc) != 0x00)return 0x06;

    //reset should clear both flags and drop len counter to 0
    ad_set_sample_len(0x8010);

    resp = CD_REG_180A;
    if ((rd_1803() & 0xc) != 0x00)return 0x07;

    CD_REG_180D = 0x80;
    asm("nop");
    CD_REG_180D = 0x00;

    if ((rd_1803() & 0xc) != 0x00)return 0x08;
    resp = CD_REG_180A;
    if ((rd_1803() & 0xc) != 0x08)return 0x09;

    ad_set_sample_len(0);
    CD_REG_180D = 0x80 | 0x60; //reset and play
    gVsync();
    //busy flag should be there even during the reset
    if ((rd_180C() & 0x8) != 0x08)return 0x10;
    CD_REG_180D = 0;
    gVsync();

    //reset should set wr and rd addr to 0
    for (i = 0; i < 256; i++) {
        cbuff[i] = i;
    }
    ad_ram_wr(cbuff, 0, 256);
    ad_set_addr_rd(0);
    ad_set_addr_wr(128);


    CD_REG_180D = 0x80;
    asm("nop");
    CD_REG_180D = 0x00;
    gVsync();

    //check read address
    ad_byte_rd();
    if (ad_byte_rd() != 0x00)return 0x11;
    if (ad_byte_rd() != 0x01)return 0x11;
    if (ad_byte_rd() != 0x02)return 0x11;

    //check write address
    ad_byte_wr(0x2A);
    ad_byte_wr(0x55);
    ad_set_addr_rd(0);
    ad_byte_rd();
    if (ad_byte_rd() != 0x2A)return 0x12;
    if (ad_byte_rd() != 0x55)return 0x12;


    return 0;
}

u8 tst_ad_ram() {

    u16 i, u;

    ad_reset();

    ad_set_addr_wr(0);
    //test single 8B block
    for (i = 0; i < 8; i++) {
        ad_byte_wr(i);
    }

    ad_set_addr_rd(0);
    ad_byte_rd(); //put first byte to the buffer
    for (i = 0; i < 8; i++) {
        if (ad_byte_rd() != i)return 0x01;
    }

    //test 256 bytes
    ad_set_addr_wr(0);
    for (i = 0; i < 256; i++) {
        ad_byte_wr(i);
    }

    ad_set_addr_rd(0);
    ad_byte_rd(); //put first byte to the buffer
    for (i = 0; i < 256; i++) {
        if (ad_byte_rd() != i)return 0x02;
    }


    //everything below was due the specific address set sequence
    //seems some buggy or unknown behaviour break access to whole ram
    //only 32k works. 32K banks randomly switch from one to other
    //first test seems wtite to one 32K bank, rest tests to the another

    //test whole ram (32K?)
    for (u = 0; u < 65536 / 16; u++) {

        ad_set_addr_wr(u * 16);
        ad_byte_wr(u);
        ad_byte_wr(u >> 8);
    }


    for (u = 0; u < 65536 / 16; u++) {

        ad_set_addr_rd(u * 16);
        ad_byte_rd(); //put first byte to the buffer

        if (ad_byte_rd() != (u & 0xff)) {
            return 0x03;
        }
        if (ad_byte_rd() != (u >> 8)) {
            return 0x03;
        }
    }

    return 0;
}
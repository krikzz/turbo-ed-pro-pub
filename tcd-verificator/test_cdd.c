
#include "main.h"



u8 tst_cdd_toc();
u8 tst_cdd_rd_cpu();
u8 tst_cdd_rd_dma1();
u8 tst_cdd_rd_dma2();
u8 tst_cdd_irq(u8 init);
u8 tst_cdd_status1();
u8 tst_cdd_status2();
u8 tst_cdd_rdbuff();
u8 tst_cdd_dma_time();
u8 tst_cdd_cmd_time();
u8 tst_cdd_var();

u8 tst_cdd() {

    u8 resp;

    gConsPrint("init cd");
    resp = cd_init();
    printResp(resp);
    if (resp)return 1;

    gConsPrint("cdd status1");
    resp = tst_cdd_status1();
    printResp(resp);

    gConsPrint("cdd status2");
    resp = tst_cdd_status2();
    printResp(resp);

    gConsPrint("cdd buffer");
    resp = tst_cdd_rdbuff();
    printResp(resp);

    gConsPrint("cdd irq");
    resp = tst_cdd_irq(0);
    printResp(resp);

    gConsPrint("read toc");
    resp = tst_cdd_toc();
    printResp(resp);

    gConsPrint("read cd cpu");
    resp = tst_cdd_rd_cpu();
    printResp(resp);

    gConsPrint("read cd dma1");
    resp = tst_cdd_rd_dma1();
    printResp(resp);

    gConsPrint("read cd dma2");
    resp = tst_cdd_rd_dma2();
    printResp(resp);

    gConsPrint("cdd dma timings");
    resp = tst_cdd_dma_time();
    printResp(resp);

    gConsPrint("cdd cmd timings");
    resp = tst_cdd_cmd_time();
    printResp(resp);

    gConsPrint("cdd var");
    resp = tst_cdd_var();
    printResp(resp);

    return 0;
}

u8 tst_cdd_toc() {

    u32 tnum;
    u32 dsiz;
    u32 inf[4];
    u16 i;
    u16 resp;

    resp = cd_cmd_toc(&tnum, TOC_TRACKS);
    if (resp)return 1;
    resp = cd_cmd_toc(&dsiz, TOC_DISK_SIZE);
    if (resp)return 2;

    for (i = 0; i < 4; i++) {
        resp = cd_cmd_toc(&inf[i], TOC_TRACK_INF | (i + 1));
        if (resp)return 3;
    }

    if (tnum != 0x00006001)return 4;
    if (dsiz != 0x00000549)return 5;
    if (inf[0] != 0x00000200)return 6;
    if (inf[1] != 0x04654900)return 7;
    if (inf[2] != 0x00043801)return 8;
    if (inf[3] != 0x00443204)return 9;

    /*
    gConsPrint("tra: ");
    gAppendHex32(tnum);
    gConsPrint("siz: ");
    gAppendHex32(dsiz);

    for (i = 0; i < 4; i++) {
        gConsPrint("inf: ");
        gAppendHex32(inf[i]);
    }*/

    return 0;
}

u8 tst_cdd_rd_cpu() {

    u16 i;
    u16 resp;
    u32 crc;
    u8 *wram = (u8 *) ADDR_RAM_BANK;
    u8 bank = sysGetBank();

    CD_REG_180B = 0x00; //make sure adpcm dma turned off

    //try read 1 sector.
    for (i = 0; i < 4096; i++) {
        cbuff[i] = 0xff;
    }

    resp = cd_cmd_read_mem(cbuff, SECTOR_DATA, 1);
    if (resp)return 1;

    crc = crc32(0, cbuff, 2048);
    if (crc != SECTOR_CRC1)return 2;

    //only 2048 bytes should be readed. make sure data above 2048 is not modified
    for (i = 0; i < 2048; i++) {
        if (cbuff[i + 2048] != 0xff)return 3;
    }

    //try read 2 sectors
    resp = cd_cmd_read_mem(cbuff, SECTOR_DATA, 2);
    if (resp)return 4;

    crc = crc32(0, cbuff, 2048);
    if (crc != SECTOR_CRC1)return 5;

    crc = crc32(0, cbuff + 2048, 2048);
    if (crc != SECTOR_CRC2)return 6;

    for (i = 0; i < 4096; i++) {
        cbuff[i] = 0xff;
    }

    //try read 2 sectors via 1801
    resp = cd_cmd_read_mem_1801(cbuff, SECTOR_DATA, 2);
    if (resp)return 7;

    crc = crc32(0, cbuff, 2048);
    if (crc != SECTOR_CRC1)return 8;

    crc = crc32(0, cbuff + 2048, 2048);
    if (crc != SECTOR_CRC2)return 9;


    cd_cmd_read(SECTOR_DATA, 32);
    for (i = 0; i < 32; i += 4) {
        sysSetBank(TCD_BANK_WRAM + i / 4);
        cd_tx_sectors(wram, 4);
    }
    resp = cd_read_resp();
    if (resp) {
        sysSetBank(bank);
        return 0x10;
    }

    crc = 0;
    for (i = 0; i < 32; i += 4) {
        sysSetBank(TCD_BANK_WRAM + i / 4);
        crc = crc32(crc, (u8 *) ADDR_RAM_BANK, 8192);
    }
    sysSetBank(bank);

    if (crc != SECTOR_CRC64K) {

        u32 crc_old = crc;
        crc = 0;
        for (i = 0; i < 32; i += 4) {
            sysSetBank(TCD_BANK_WRAM + i / 4);
            crc = crc32(crc, (u8 *) ADDR_RAM_BANK, 8192);
        }
        sysSetBank(bank);

        if (crc == SECTOR_CRC64K) {
            return 0x12; //single error during ram read
        }

        if (crc != crc_old) {
            return 0x13; //multiple error during ram read
        }

        return 0x14; //error during ram write
    }

    sysSetBank(bank);

    /*
    gConsPrint("crc: ");
    gAppendHex32(crc);*/
    return 0;
}

u8 tst_cdd_rd_dma1() {

    u32 i;
    u16 resp;
    u32 crc;

    //try read 1 sector.
    for (i = 0; i < 4096; i++) {
        cbuff[i] = 0xff;
    }
    ad_ram_wr(cbuff, 0, 4096);

    resp = cd_cmd_read_dma(0, SECTOR_DATA, 1);
    if (resp)return 1;
    ad_ram_rd(cbuff, 0, 4096);

    crc = crc32(0, cbuff, 2048);
    if (crc != SECTOR_CRC1)return 2;

    //only 2048 bytes should be readed. make sure data above 2048 is not modified
    for (i = 0; i < 2048; i++) {
        if (cbuff[i + 2048] != 0xff)return 3;
    }

    //try read 2 sectors
    resp = cd_cmd_read_dma(0, SECTOR_DATA, 2);
    if (resp)return 4;
    ad_ram_rd(cbuff, 0, 4096);

    crc = crc32(0, cbuff, 2048);
    if (crc != SECTOR_CRC1)return 5;

    crc = crc32(0, cbuff + 2048, 2048);
    if (crc != SECTOR_CRC2)return 6;



    return 0;
}

u8 tst_cdd_rd_dma2() {

    u16 i;
    u32 crc;
    u16 resp;

    ad_reset();

    //pcm dma during playback
    CD_REG_180D = 0x00;
    for (i = 0; i < 4096; i++) {
        cbuff[i] = 0xaa;
    }
    ad_ram_wr(cbuff, 0, 4096);
    ad_set_addr_wr(0);
    ad_set_addr_rd(0);

    resp = cd_cmd_read(SECTOR_DATA, 2);
    if (resp)return 1;
    cd_dat_busy();

    CD_REG_180E = 12; //rate
    CD_REG_180D = 0x20; //infinity pcm playback
    CD_REG_180B = 0x02; // run dma
    ad_dma_busy();
    CD_REG_180B = 0x00; //dma off
    resp = cd_read_resp();
    CD_REG_180D = 0x00; //pcm stop
    if (resp)return 2;

    ad_ram_rd(cbuff, 0, 4096);
    crc = crc32(0, cbuff, 2048);
    if (crc != SECTOR_CRC1)return 3;

    crc = crc32(0, cbuff + 2048, 2048);
    if (crc != SECTOR_CRC2)return 4;

    //dma should inc len counter, just like writes via 180A
    ad_set_sample_len(32768L - 2048);
    cd_cmd_read_dma(0, SECTOR_DATA, 1);
    if ((rd_1803() & 0xC) != 4)return 0x10;
    ad_byte_wr(0);
    if ((rd_1803() & 0xC) != 0)return 0x11;

    //one more byte in sample len
    ad_set_sample_len(32768L - 2048 + 1);
    cd_cmd_read_dma(0, SECTOR_DATA, 1);
    if ((rd_1803() & 0xC) != 0)return 0x12;


    //read 2 sectors
    ad_set_sample_len(32768L - 2048);
    cd_cmd_read_dma(0, SECTOR_DATA, 2);
    if ((rd_1803() & 0xC) != 0)return 0x13;

    //read whole 32K
    ad_set_sample_len(0);
    cd_cmd_read_dma(0, SECTOR_DATA, 16);
    if ((rd_1803() & 0x4) != 0x4)return 0x14;
    if ((rd_1803() & 0xC) != 0xc)return 0x15; //end bit should be set also cuz dma start from zero len
    ad_byte_wr(0);
    if ((rd_1803() & 0x4) != 0x0)return 0x16;
    if ((rd_1803() & 0xC) != 0x8)return 0x17; //end bit should be set also cuz dma start from zero len


    //test block by block dma
    //180B:0 read single block then turn off automatically
    //180B:0 can be set only if data ready for read
    memSet(cbuff, 0xff, 2048);
    ad_ram_wr(cbuff, 0, 2048);

    ad_set_addr_wr(0);
    CD_REG_180B = 0;
    resp = cd_cmd_read(SECTOR_DATA, 2);
    if (resp)return 0x20;
    cd_dat_busy();

    //read first sector
    //CD_REG_180B |= 1;
    cd_reg_w(0x180b, 1);
    if (cd_reg_r(0x180b) != 1)return 0x21;
    i = 0;
    while (cd_reg_r(0x180b) == 1) {
        gVsync();
        if (i++ >= 30)return 0x22; //timeout
    }
    ad_ram_rd(cbuff, 0, 2048);
    crc = crc32(0, cbuff, 2048);
    //gPrintHex(cbuff, 64);
    //gConsPrint("");
    //gPrintHex(cbuff + 2048 - 256, 256);
    if (crc != SECTOR_CRC1)return 0x23;

    //read second sector
    ad_set_addr_wr(0);
    cd_reg_w(0x180b, 1);
    if (cd_reg_r(0x180b) != 1)return 0x24;
    i = 0;
    while (cd_reg_r(0x180b) == 1) {
        gVsync();
        if (i++ >= 16)return 0x25; //timeout
    }
    ad_ram_rd(cbuff, 0, 2048);
    crc = crc32(0, cbuff, 2048);
    if (crc != SECTOR_CRC2)return 0x26;

    resp = cd_read_resp();
    if (resp)return 0x27;


    //test block dma with partial read via 1808
    //last 128 bytes shouldn't be modified
    memSet(cbuff, 0x2a, 4096);
    ad_ram_wr(cbuff, 0, 4096);

    ad_set_addr_wr(0);
    CD_REG_180B = 0;
    resp = cd_cmd_read(SECTOR_DATA, 2);
    if (resp)return 0x30;
    cd_dat_busy();

    for (i = 0; i < 128; i++) {
        cd_reg_r(0x1808);
    }
    cd_reg_w(0x180b, 1);
    while (cd_reg_r(0x180b) == 1);

    ad_ram_rd(cbuff, 0, 2048);
    crc = crc32(0, cbuff, 2048);
    if (crc != 0x942072E4)return 0x31;

    cd_reg_w(0x180b, 1);
    while (cd_reg_r(0x180b) == 1);
    resp = cd_read_resp();
    if (resp)return 0x32;

    ad_ram_rd(cbuff, 0, 4096);
    crc = crc32(0, cbuff, 2048);
    if (crc != 0xE3E2A682)return 0x33;
    crc = crc32(0, cbuff, 4096);
    if (crc != 0x6DEA9F9B)return 0x34;

    //ad_byte_wr(0);
    return 0;
}

u8 tst_cdd_irq(u8 init) {

    u16 i;
    u16 resp;
    vu8 tmp;

    if (init == 0) {

        CD_REG_180B = 0x00; //make sure adpcm dma turned off
        sysIrqON();
        resp = tst_cdd_irq(1);
        sysIrqOFF();
        CD_REG_180B = 0x00;
        return resp;
    }

    //cd_cmd_end();


    //turn off all irq. irq shoudn't happen
    cd_cmd_read(SECTOR_DATA, 1);
    CD_REG_1802 = 0x00; //turn off any irq
    irq_ctr = 0;
    cd_dat_busy();
    gVsync();
    if (irq_ctr != 0)return 1;
    for (i = 0; i < 2048; i++) {
        tmp = CD_REG_1808;
    }
    cd_dat_busy();
    gVsync();
    if (irq_ctr != 0)return 2;
    resp = cd_read_resp();
    //cd_start_cmd();
    if (resp)return 3;

    //gConsPrint("irq ctr: ");
    //gAppendHex8(cd_status());

    //check data start and data end irq
    cd_cmd_read(SECTOR_DATA, 1);
    CD_REG_1802 = 0x40; //data start irq
    irq_ctr = 0;
    cd_dat_busy();
    if (irq_ctr != 1)return 3;
    CD_REG_1802 = 0x40; //re enable d-start irq should trigger irq instantly
    asm("nop");
    if (irq_ctr != 2)return 3;

    CD_REG_1802 = 0x20; //data end irq
    for (i = 0; i < 2047; i++) {
        tmp = CD_REG_1808;
    }
    cd_dat_busy();
    gVsync();
    if (irq_ctr != 2)return 5; //one byte still remains. no irq

    tmp = CD_REG_1808; //read last byte
    //while (cd_status() != 0xd8);
    cd_dat_busy();
    if (irq_ctr != 3)return 6; //all bytes readed. irq should be triggered

    CD_REG_1802 = 0x20; //re enable d-end irq should trigger irq instantly
    asm("nop");
    if (irq_ctr != 4)return 7;

    CD_REG_1802 = 0x40; //re enable d-start shoudn't have any effect if no data for read
    asm("nop");
    if (irq_ctr != 4)return 8;

    resp = cd_read_resp();
    //cd_start_cmd();
    if (resp)return 9;

    CD_REG_1802 = 0x60; //enable both irq
    gVsync();
    if (irq_ctr != 4)return 0x0A; //irq shouldn't happen after cmd end



    //irq should happen only in the end
    cd_cmd_read(SECTOR_DATA, 1);
    CD_REG_1802 = 0x20; //turn on d-end irq
    irq_ctr = 0;
    cd_dat_busy();
    gVsync();
    if (irq_ctr != 0)return 0x10;
    for (i = 0; i < 2047; i++) {
        tmp = CD_REG_1808;
    }
    cd_dat_busy();
    gVsync();
    if (irq_ctr != 0)return 0x11;
    tmp = CD_REG_1808; //read last byte
    //while (cd_status() != 0xd8);
    cd_dat_busy();
    if (irq_ctr != 1)return 0x12;

    resp = cd_read_resp();
    //cd_start_cmd();
    if (resp)return 0x13;

    //try to read 2 sectors. irq should be only in the end of last sector
    cd_cmd_read(SECTOR_DATA, 2);
    CD_REG_1802 = 0x20; //turn on d-end irq
    irq_ctr = 0;
    cd_dat_busy();
    gVsync();
    if (irq_ctr != 0)return 0x21;
    for (i = 0; i < 2048; i++) {
        tmp = CD_REG_1808;
    }
    cd_dat_busy();
    gVsync();
    if (irq_ctr != 0)return 0x22;

    for (i = 0; i < 2048; i++) {
        tmp = CD_REG_1808;
    }
    cd_dat_busy();
    if (irq_ctr != 1)return 0x23; //irq should fire after last byte in last sector

    resp = cd_read_resp();
    //cd_start_cmd();
    if (resp)return 0x24;


    //irq should work for dma also
    irq_ctr = 0;
    cd_cmd_read(SECTOR_DATA, 2);
    CD_REG_1802 = 0x20; //turn on d-end irq
    CD_REG_180B = 0x02; //run dma
    while (cd_status() != 0xD8);

    if (irq_ctr != 1)return 0x30;
    resp = cd_read_resp();
    //cd_start_cmd();
    if (resp)return 0x31;


    //irq should trigger after last byte and after first resp byte
    CD_REG_180B = 0;
    CD_REG_1802 = 0x00;
    irq_ctr = 0;
    cd_cmd_read(0x0e06, 1);
    cd_dat_busy();

    for (i = 0; i < 2047; i++) {
        tmp = CD_REG_1808;
    }

    //no irq here. last byte not readed yet
    CD_REG_1802 = 0x20;
    cd_dat_busy();
    if (irq_ctr != 0)return 0x40;

    //irq should trigger. first resp byte in buffer
    tmp = CD_REG_1808;
    cd_dat_busy();
    if (irq_ctr != 1)return 0x41;

    //irq should trigger. second resp byte in buffer
    cd_dat_rd();
    CD_REG_1802 = 0x20;
    cd_dat_busy();
    if (irq_ctr != 2)return 0x42;

    //irq shouldn't trigger. no any remain bytes for read
    cd_dat_rd();
    CD_REG_1802 = 0x20;
    gVsync();
    if (irq_ctr != 2)return 0x43;

    CD_REG_1802 = 0x00;
    //cd_start_cmd();

    return 0;
}

u8 tst_cdd_status1() {

    u16 i;
    u8 tmp;

    CD_REG_180B = 0x00; //make sure adpcm dma turned off

    cd_cmd_read(SECTOR_DATA, 1);
    cd_dat_busy();
    //gConsPrint("xx: ");
    //gAppendHex8(cd_status());
    //gConsPrint("");
    if (cd_status() != 0xC8)return 1; //0xc8 if can read data

    for (i = 0; i < 2047; i++) {
        tmp = CD_REG_1808;
    }

    cd_dat_busy();
    if (cd_status() != 0xC8)return 2; //still one byte in remains

    tmp = CD_REG_1808;
    cd_dat_busy();
    if (cd_status() != 0xD8)return 3; //all data readed. first resp byte
    cd_dat_rd();
    cd_dat_busy();
    if (cd_status() != 0xF8)return 4; //second resp byte
    cd_dat_rd();
    gVsync();
    gVsync();
    if (cd_status() != 0x00)return 5; //cmd complete. status should be 00

    cd_start_cmd(); //prepare to the next command
    cd_dat_busy();

    if (cd_status() != 0xD0)return 6; //system ready to receive cmd bytes

    //tx cmd 0
    cd_dat_wr(0);
    cd_dat_wr(0);
    cd_dat_wr(0);
    cd_dat_wr(0);
    cd_dat_wr(0);

    cd_dat_busy();
    if (cd_status() != 0xD0)return 7; //system ready to receive cmd bytes

    cd_dat_wr(0); //last cmd byte

    //now read resp
    cd_dat_busy();
    if (cd_status() != 0xD8)return 9; //all data readed. first resp byte
    cd_dat_rd();
    cd_dat_busy();
    if (cd_status() != 0xF8)return 9; //second resp byte
    cd_dat_rd();
    gVsync();
    gVsync();
    if (cd_status() != 0x00)return 0x10; //cmd complete. status should be 00

    //cd_start_cmd(); //prepare to the next command


    cd_cmd_read(SECTOR_DATA, 1);
    cd_dat_busy();
    for (i = 0; i < 2048; i++) {
        tmp = CD_REG_1808;
    }

    //try read resp bytes via CD_REG_1808
    cd_dat_busy();
    if (cd_status() != 0xD8)return 0x11; //all data readed. first resp byte

    tmp = CD_REG_1808;
    cd_dat_busy();
    gVsync();
    if (cd_status() != 0xD8)return 0x12; //read via 1808 shouldn't affect resp bytes
    tmp = CD_REG_1808;
    cd_dat_busy();
    gVsync();
    if (cd_status() != 0xD8)return 0x12; //read via 1808 shouldn't affect resp bytes

    cd_dat_rd();
    cd_dat_busy();
    if (cd_status() != 0xF8)return 0x13;
    cd_dat_rd();
    //cd_start_cmd();


    return 0;
}

u8 tst_cdd_status2() {

    u16 i;
    u8 tmp;
    u16 resp;

    //set 1803:1 to the 0
    for (i = 0; i < 64 && (CD_REG_1803 & 2) != 0; i++) {
        CD_REG_1805 = 0;
    }

    //1803:2/3 should set after dma if sample len 0
    ad_set_sample_len(0x0000);

    ad_set_addr_wr(0);
    CD_REG_180B = 0; //turn off dma
    CD_REG_180D = 0x00;

    //status
    resp = cd_cmd_read(SECTOR_DATA, 1);
    if (resp)return 0x01;
    if (CD_REG_180C != 0x00)return 0x02;
    if (CD_REG_1803 != 0x10)return 0x03;
    cd_dat_busy();
    if (CD_REG_180C != 0x00)return 0x04;
    if (CD_REG_1803 != 0x50)return 0x05;
    if (CD_REG_1800 != 0xc8)return 0x06;

    for (i = 0; i < 2048; i++) {
        tmp = CD_REG_1808;
    }
    if (CD_REG_180C != 0x00)return 0x07;
    if (CD_REG_1803 != 0x10)return 0x08;

    cd_dat_busy();
    if (CD_REG_180C != 0x00)return 0x09;
    if (CD_REG_1803 != 0x30)return 0x10;
    if (CD_REG_1800 != 0xD8)return 0x11;

    cd_dat_rd();
    tmp = CD_REG_1803;
    cd_dat_busy();
    if (CD_REG_180C != 0x00)return 0x12;
    if (CD_REG_1803 != 0x30)return 0x13;
    if (CD_REG_1800 != 0xF8)return 0x14;
    //bit 5 should be set only if resp bytes ready for read (1800== D8 or F8)
    //if (tmp != 0x10)return 0x15;

    cd_dat_rd();
    while (cd_status() != 0);
    if (CD_REG_180C != 0x00)return 0x16;
    if (CD_REG_1803 != 0x10)return 0x17;
    if (CD_REG_1800 != 0x00)return 0x18;

    resp = cd_cmd_read(SECTOR_DATA, 1);
    if (resp)return 0x20;

    CD_REG_180B = 2;
    while ((CD_REG_1803 & 0x20) == 0);
    CD_REG_180B = 0;
    if (CD_REG_180C != 0x01)return 0x21;
    if (CD_REG_1803 != 0x3c)return 0x22;
    if (CD_REG_1800 != 0xd8)return 0x23;

    cd_dat_rd();
    cd_dat_busy();
    if (CD_REG_180C != 0x01)return 0x24;
    if (CD_REG_1803 != 0x3c)return 0x25;
    if (CD_REG_1800 != 0xF8)return 0x26;

    cd_dat_rd();
    while (CD_REG_1800 != 0);
    if (CD_REG_180C != 0x01)return 0x27;
    if (CD_REG_1803 != 0x1C)return 0x28;

    return 0;
}

u8 tst_cdd_rdbuff() {

    u16 i;
    u8 tmp;

    CD_REG_180B = 0x00; //make sure adpcm dma turned off

    cd_cmd_read(SECTOR_DATA, 1);
    cd_dat_busy();

    //read from 1808 push new byte to the 1801 and 1808
    //during the read op, both regs show data from cdd
    //during read op writes to 1801 still modify internal latch and value will be exposed after rd end
    //write to the 1808 shoudn't have any affect
    //test listed above behaviors

    if (cd_reg_r(0x1801) != SECTOR_VAL0)return 1;
    cd_reg_w(0x1801, SECTOR_VAL0 ^ 0xff);
    cd_reg_w(0x1802, SECTOR_VAL0 ^ 0xff);
    if (cd_reg_r(0x1801) != SECTOR_VAL0)return 2;
    if (cd_reg_r(0x1808) != SECTOR_VAL0)return 3; //read 1808 and push new byte

    if (cd_reg_r(0x1801) != SECTOR_VAL1)return 4;
    if (cd_reg_r(0x1808) != SECTOR_VAL1)return 5;

    //read remain bytes
    for (i = 0; i < 2048 - 2; i++) {
        tmp = CD_REG_1808;
    }


    //still should be read only till resp bytes remains
    tmp = cd_reg_r(0x1801);
    cd_reg_w(0x1801, tmp ^ 0xff);
    if (cd_reg_r(0x1801) != tmp)return 6;


    cd_reg_w(0x1801, 0x2A); //modify internal latch
    if (cd_reg_r(0x1801) != 0)return 7; //normal resp should be 0

    cd_read_resp();
    gVsync();
    gVsync();

    //check internal latch
    if (cd_reg_r(0x1801) != 0x2A)return 8;
    if (cd_reg_r(0x1808) != 0x2A)return 9;

    //should be writable now
    tmp = cd_reg_r(0x1801);
    cd_reg_w(0x1801, tmp ^ 0xff);
    if (cd_reg_r(0x1801) != (tmp ^ 0xff))return 0x10;
    if (cd_reg_r(0x1808) != (tmp ^ 0xff))return 0x11; //1808 should mirro 1808

    //cd_start_cmd();

    //still should be writable
    tmp = cd_reg_r(0x1801);
    cd_reg_w(0x1801, tmp ^ 0xff);
    if (cd_reg_r(0x1801) != (tmp ^ 0xff))return 0x12;
    if (cd_reg_r(0x1808) != (tmp ^ 0xff))return 0x13; //1808 should mirro 1808
    if (cd_reg_r(0x1808) != (tmp ^ 0xff))return 0x14; //previous 1808 read soudn't push new data

    return 0;
}

u8 tst_cdd_dma_time() {

    u32 i;
    u16 time;
    //check dma time
    i = 0;
    cd_cmd_read(SECTOR_DATA, 1);
    cd_dat_busy();
    ad_dma_start(0);
    time = get_ticks();
    while (cd_status() != 0xD8) {
        i++;
    }
    time = get_ticks() - time; //should be 8us
    cd_read_resp();
    cd_start_cmd();

    //gConsPrint("time: ");
    //gAppendNum(i);

    if (i >= 207 + 20)return 7; //too long
    if (i <= 207 - 20)return 8; //too short


    //check dma time for multisector. 
    i = 0;
    cd_cmd_read(SECTOR_DATA, 16);
    cd_dat_busy();
    ad_dma_start(0);
    time = get_ticks();
    while (cd_status() != 0xD8) {
        i++;
    }
    time = get_ticks() - time; //should be 8us + 13.3*sector num
    cd_read_resp();
    //cd_start_cmd();

    //gConsPrint("time: ");
    //gAppendNum(i);

    if (i >= 5548 + 32)return 0x10; //too long
    if (i <= 5548 - 32)return 0x12; //too short

    //dma should star even if enabled befor disk read
    CD_REG_180B = 0x02;
    cd_cmd_read(SECTOR_DATA, 1);
    cd_dat_busy();
    i = 0;
    while (cd_status() != 0xD8) {
        i++;
        gVsync();
        if (i > 30)return 0x13;
    }
    cd_read_resp();
    //cd_start_cmd();

    /*
    gConsPrint("i: ");
    gAppendNum(i);

    gConsPrint("t: ");
    gAppendNum(time);*/

    return 0;
}

u16 dclk_time(u8 val, u8 stat) {

    u16 time = 0;

    CD_REG_1801 = val;
    CD_REG_1802 = 0x80;
    asm("nop");
    CD_REG_1802 = 0x00;

    while (CD_REG_1800 != stat) {
        time++;
    }

    return time;
}

u8 tst_cdd_cmd_time() {

    u16 time_db[16];
    u16 time;
    u32 tmp;
    u16 i;
    u16 resp;
    u8 cmd;
    u8 stat[] = {
        0xd0, 0xd0, 0xd0, 0xd0, 0xd0, 0xd0, 0xd0, 0xd0, 0xd0,
        0xc8, 0xc8, 0xc8, 0xc8,
        0xd8, 0xf8, 0x00
    };

    /*
    gConsPrint("init...");
    cd_init();
    gAppendString("ok");*/
    cd_init();
    gVsync();
    for (i = 0; i < 8; i++) {
        cd_cmd_toc(&tmp, 0);
        gVsync();
    }


    //test cmd start time
    resp = cd_cmd_toc(&tmp, 0);
    if (resp)return 1;
    gVsync();
    time = 0;
    CD_REG_1801 = 0x80; //cmd start
    CD_REG_1800 = 0x81;
    while (CD_REG_1800 != 0xd0) {
        time++;
    }
    //gConsPrint("tm: ");
    //gAppendNum(time);
    if (time < 20)return 2; //too fast. 46
    if (time > 800)return 3; //too slow. 700

    //test bytes rx/tx time
    //regular time:
    //20 first byte tx
    //05 remain bytes tx
    //20 first byte rx
    //03 remain bytes rx (3-4)
    //12 first resp byte
    //05 sec resp byte
    //05 go to idle
    for (i = 0; i < 16; i++) {
        cmd = i == 0 ? CD_CMD_TOC : 0;
        time_db[i] = dclk_time(cmd, stat[i]);
    }
    /*
    gCleanScreen();
    for (i = 0; i < 16; i++) {
        gConsPrint("tm: ");
        gAppendHex8(i);
        gAppendChar('.');
        gAppendHex8(stat[i]);
        gAppendChar('.');
        gAppendNum(time_db[i]);
    }
    gConsPrint("");*/

    if (time_db[0x00] < 10)return 4; //first cmd byte
    if (time_db[0x09] < 10)return 5; //first data byte
    if (time_db[0x0D] < 8)return 6; //first resp byte

    /*
    gConsPrint("");
    gPrintHex(time_db[i],32);
    gConsPrint("");*/

    for (i = 0; i < 16; i++) {
        if (time_db[i] < 2)return 7; //too fast
        if (time_db[i] > 33)return 8; //to slow
    }

    return 0;
}

u8 tst_cdd_var() {

    u16 i;
    u16 resp;

    CD_REG_1801 = 0x80; //cmd start
    CD_REG_1800 = 0x81;
    while (CD_REG_1800 != 0xd0);

    for (i = 0; i < 9; i++) {
        dclk_time(i == 0 ? CD_CMD_TOC : 0, 0xd0);
    }
    dclk_time(0, 0xc8);


    //cmd interrupt
    CD_REG_1801 = 0x80; //cmd start
    CD_REG_1800 = 0x81;

    i = 0;
    while (CD_REG_1800 != 0xd0 && CD_REG_1800 != 0x00) {
        if (i++ > 0x8000)return 1; //cmd interrupt does not work
    }

    CD_REG_1801 = 0xff;
    //cd_dat_wr(0xff); //reuire for proper cmd termination. delay also works

    resp = cd_cmd_play_D8(3, PMOD_D8_SEEK, AMOD_TRA);
    if (resp)return 2;

    return 0;
}
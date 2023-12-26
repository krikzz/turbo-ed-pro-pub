
#include "main.h"

#define CDC_ST_EXEC     0x08 //1 if cmd executed
#define CDC_ST_RESP     0x10 //0 if data byte in buff
#define CDC_ST_LAST     0x20 //1 if last status byte
#define CDC_ST_DRDY     0x40 //1 if can read/write
#define CDC_ST_LISN     0x80 //1 if cdc listen

void cd_tx_sectors_asm();
void cd_tx_sectors_asm_sw();

u8 rd_1803() {

    u8 i;
    for (i = 0; i < 4; i++) {
        asm("nop");
        asm("nop");
    }

    return CD_REG_1803;
}

u8 rd_180C() {

    u8 i;
    for (i = 0; i < 4; i++) {
        asm("nop");
        asm("nop");
    }

    return CD_REG_180C;
}

void cd_reg_w(u16 reg, u8 val) {

    *((u8 *) reg) = val;
    /*
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");*/
    //gVsync();
}

u8 cd_reg_r(u16 reg) {

    //gVsync();
    /*asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");*/
    return *((u8 *) reg);
}

u8 cd_status() {
    return CD_REG_1800;
}

void cd_dat_wr(u8 val) {

    //vu8 tmp;

    //while ((cd_status() & 0x40) != 0x40);
    cd_dat_busy();

    CD_REG_1801 = val;
    CD_REG_1802 = 0x80;
    //CD_REG_1801 = 0xff;
    asm("nop");
    CD_REG_1802 = 0x00;

    /*
    CD_REG_1801 = 0xff;
    CD_REG_1802 = 0x80;
    asm("nop");
    CD_REG_1802 = 0x00;*/

    /*
    CD_REG_1801 = val;
    asm("nop");
    tmp = CD_REG_1802;
    asm("nop");
    CD_REG_1802 = 0x80;
    asm("nop");
    tmp = CD_REG_1800;
    asm("nop");
    tmp = CD_REG_1802;
    asm("nop");
    CD_REG_1802 = 0x00;
    asm("nop");*/
}

u8 cd_dat_busy() {//wait till 1801 will be ready for read/write

    u8 stat = 0;
    while ((stat & 0x40) != 0x40) {
        stat = cd_status();
    }

    return stat;
}

u8 cd_dat_rd() {

    //vu8 tmp;
    u8 val;

    cd_dat_busy();

    val = CD_REG_1801;
    //val = TCD_REG_1808;
    //asm("nop");
    //tmp = CD_REG_1802;
    //asm("nop");
    CD_REG_1802 = 0x80;
    //asm("nop");
    //tmp = CD_REG_1800;
    //asm("nop");
    //tmp = CD_REG_1802;
    //asm("nop");
    CD_REG_1802 = 0x00;
    //asm("nop");
    return val;
}

u16 cd_tx_cmd(u8 *cmd, u8 len) {

    u8 status;

    cd_start_cmd();

    while (len--) {

        status = cd_dat_busy();

        //abort if status has unexpected value
        if (status != 0xD0) {

            u16 resp = CD_ERR_UNXP_STAT;

            //read cmd resp if it there
            if (resp == 0xD8) {
                resp = cd_read_resp();
                resp |= CD_ERR_CMD_ABORT;
            }
            return resp;
        }

        cd_dat_wr(*cmd++);
    }

    return 0;
}

u16 cd_read_resp() {

    u16 resp = 0;

    if (cd_dat_busy() != 0xD8) {
        /*
         resp = cd_dat_busy();
         gConsPrint("stat: ");
         gAppendHex16(resp);
         gAppendChar('.');*/

        return CD_ERR_UNXP_STAT;
    }

    resp |= cd_dat_rd() << 8;
    resp |= cd_dat_rd();

    while (cd_status() != 0);

    return resp;
}

void cd_start_cmd() {


    u8 i;

    //if (cd_status() == 0xD0)return; //del me

    while (1) {

        CD_REG_1801 = 0x80;
        CD_REG_1800 = 0x81;
        asm("nop");
        asm("nop");

        for (i = 0; i < 8; i++) {
            gVsync();
            if (cd_status() == 0xD0)return;
        }
    }

}

u16 cd_cmd_00() {

    u16 resp = 0;
    u8 cmd[6];

    cmd[0] = CD_CMD_TEST;
    cmd[1] = 0x00;
    cmd[2] = 0x00;
    cmd[3] = 0x00;
    cmd[4] = 0x00;
    cmd[5] = 0x00;

    resp = cd_tx_cmd(cmd, sizeof (cmd));
    if (resp)return resp;

    return cd_read_resp();
}

u16 cd_cmd_play_D8(u32 addr, u8 pmode, u8 amod) {//start infinity play from specific addr

    u16 resp;
    u8 cmd[10];

    //play mode: 
    //00: seek and pause
    //01: seek and play

    cmd[0] = CD_CMD_SEEK;
    cmd[1] = pmode; //_DH & 0x07. play mode

    if (amod == AMOD_TRA) {
        cmd[2] = addr; //0x30;
        cmd[3] = 0; //0x50;
        cmd[4] = 0; //0x32;
    } else {
        cmd[2] = addr >> 16; //0x30;
        cmd[3] = addr >> 8; //0x50;
        cmd[4] = addr >> 0; //0x32;
    }
    cmd[5] = 0x00; //?
    cmd[6] = 0x00; //?
    cmd[7] = 0x00; //?
    cmd[8] = 0x00; //?
    cmd[9] = amod; //_DH & 0xC0. addr type 0x40-MSF, 0x80-track num, 00-LBA?

    resp = cd_tx_cmd(cmd, sizeof (cmd));
    if (resp)return resp;

    return cd_read_resp();
}

u16 cd_cmd_play_D9(u32 addr, u8 pmode, u8 amod) {//continue play to to the specific point

    u16 resp;
    u8 cmd[10];

    //resp only in mode 3, othervise status stuck at 0x80

    //play mode: 
    //00: pause
    //01: play-loop. in the end off loop 1 byte can be readed (stat C8), then stat going to 88
    //02: play-end. resp comes after end
    //03: play-end. resp comes instantly

    cmd[0] = CD_CMD_PLAY; //cmd
    cmd[1] = pmode; //_DH & 0x07. play mode
    if (amod == AMOD_TRA) {
        cmd[2] = addr;
        cmd[3] = 0;
        cmd[4] = 0;
    } else {
        cmd[2] = addr >> 16;
        cmd[3] = addr >> 8;
        cmd[4] = addr >> 0;
    }
    cmd[5] = 0x00; //?
    cmd[6] = 0x00; //?
    cmd[7] = 0x00; //?
    cmd[8] = 0x00; //?
    cmd[9] = amod; //_DH & 0xC0. addr type 0x40-MSF, 0x80-track num, 00-LBA?

    resp = cd_tx_cmd(cmd, sizeof (cmd));
    if (resp)return resp;

    return cd_read_resp();
}

void cd_reset() {

    cd_reg_r(0x1804);
    cd_reg_w(0x1804, 0x02);
    gVsync();
    cd_reg_r(0x1804);
    cd_reg_w(0x1804, 0x00);
    gVsync();
}

u16 cd_init() {


    //game (bios?) do this sequence at startup
    cd_reset();
    cd_reg_w(0x1802, 0x00); //??
    cd_reg_w(0x180f, 0x00); //set fader and byte mode for adpcm ram 
    cd_reg_w(0x180d, 0x80); //read adpcm
    cd_reg_w(0x180d, 0x00); //read adpcm
    cd_reg_w(0x180b, 0x00); //switch off adpcm dma
    cd_reg_r(0x1802);
    cd_reg_w(0x1802, 0x00); //?
    cd_reg_w(0x180e, 0x00); //adpcm sample rate
    cd_reg_r(0x1803);
    cd_reg_r(0x1807);

    //write 0x81 to 0x1801/0x1800 always in the cmd end. 
    //cd-rom ready to receive new cmd after this seq
    cd_reg_w(0x1801, 0x81);
    cd_reg_r(0x1800);
    cd_reg_w(0x1800, 0x81);

    //return cd_cmd_00();

    while (cd_cmd_00() != 0);

    return 0;
}

u16 cd_cmd_read(u32 lba, u8 blocks) {

    u8 cmd[6];

    cmd[0] = CD_CMD_READ;
    cmd[1] = lba >> 16;
    cmd[2] = lba >> 8;
    cmd[3] = lba >> 0;
    cmd[4] = blocks;
    cmd[5] = 0x00;

    return cd_tx_cmd(cmd, 6);
}

u16 cd_cmd_read_mem(u8 *dst, u32 lba, u8 blocks) {

    u16 resp;
    u16 i;
    u8 stat;

    resp = cd_cmd_read(lba, blocks);
    if (resp)return resp;

    while (blocks--) {

        stat = cd_dat_busy();

        if (stat == 0xD8) {
            resp = cd_read_resp();
            return CD_ERR_CMD_ABORT | resp;
        }

        if (stat != 0xC8) {
            return CD_ERR_UNXP_STAT;
        }

        for (i = 0; i < 2048; i++) {
            *dst++ = CD_REG_1808;
        }
    }

    return cd_read_resp();
}

u16 cd_cmd_read_mem_1801(u8 *dst, u32 lba, u8 blocks) {

    u16 resp;
    u8 stat;
    u32 len = blocks;

    len *= 2048;

    resp = cd_cmd_read(lba, blocks);
    if (resp)return resp;

    while (len--) {

        stat = cd_dat_busy();

        if (stat == 0xD8) {
            resp = cd_read_resp();
            return CD_ERR_CMD_ABORT | resp;
        }

        if (stat != 0xC8) {
            return CD_ERR_UNXP_STAT;
        }

        *dst++ = CD_REG_1801;

        CD_REG_1802 = 0x80;
        CD_REG_1802 = 0x00;
    }

    return cd_read_resp();
}

u16 cd_cmd_read_dma(u32 ad_addr, u32 lba, u8 blocks) {

    u16 resp;

    ad_dma_start(ad_addr);

    resp = cd_cmd_read(lba, blocks);
    if (resp)return resp;

    ad_dma_busy();
    CD_REG_180B = 0x00;


    return cd_read_resp();
}

u16 cd_cmd_pause() {

    u8 cmd[10];
    u16 resp = 0;

    cmd[0] = CD_CMD_PAUSE;
    cmd[1] = 0x00;
    cmd[2] = 0x00;
    cmd[3] = 0x00;
    cmd[4] = 0x00;
    cmd[5] = 0x00;
    cmd[6] = 0x00;
    cmd[7] = 0x00;
    cmd[8] = 0x00;
    cmd[9] = 0x00;

    resp = cd_tx_cmd(cmd, sizeof (cmd));
    if (resp)return resp;

    return cd_read_resp();
}

u16 cd_cmd_toc(void *toc, u16 arg) {

    u16 resp = 0;
    u8 i;
    u8 cmd[10];
    u8 *data = (u8 *) toc;

    cmd[0] = CD_CMD_TOC; //cmd 
    cmd[1] = arg >> 8; //inf type
    cmd[2] = arg >> 0; // track num (for inf type 2-3)
    cmd[3] = 0x00;
    cmd[4] = 0x00;
    cmd[5] = 0x00;
    cmd[6] = 0x00;
    cmd[7] = 0x00;
    cmd[8] = 0x00;
    cmd[9] = 0x00;

    resp = cd_tx_cmd(cmd, sizeof (cmd));
    if (resp)return resp;

    for (i = 0; i < 4; i++) {

        if (cd_dat_busy() != 0xC8) {
            return cd_read_resp();
        }

        *data++ = cd_dat_rd();
    }

    return cd_read_resp();
}

u16 cd_cmd_read_subq(Subq *subq) {

    u16 resp;
    u8 *ptr8 = (u8 *) subq;
    u8 i;

    u8 cmd[10];
    cmd[0] = CD_CMD_RDSUB;
    cmd[1] = 0x0A;
    cmd[2] = 0x00;
    cmd[3] = 0x00;
    cmd[4] = 0x00;
    cmd[5] = 0x00;
    cmd[6] = 0x00;
    cmd[7] = 0x00;
    cmd[8] = 0x00;
    cmd[9] = 0x00;

    resp = cd_tx_cmd(cmd, sizeof (cmd));
    if (resp)return resp;

    for (i = 0; i < 10; i++) {
        *ptr8++ = cd_dat_rd();
    }

    return cd_read_resp();
}

u16 cd_print_subq() {

    u8 i;
    Subq sub;
    u16 resp;

    resp = cd_cmd_read_subq(&sub);
    gConsPrint("pstat:");
    gAppendHex8(sub.play_stat);
    gConsPrint("ctrl :");
    gAppendHex8(sub.ctrl);
    gConsPrint("tnum :");
    gAppendHex8(sub.track_num);
    gConsPrint("index:");
    gAppendHex8(sub.index);

    gConsPrint("msf-t:");
    for (i = 0; i < 3; i++) {
        gAppendHex8(sub.msf_in_track[i]);
    }

    gConsPrint("msf-d:");
    for (i = 0; i < 3; i++) {
        gAppendHex8(sub.msf_in_disk[i]);
    }

    //gAppendChar('.');
    //gAppendHex8(CD_REG_1807);

    return resp;
}

u16 cd_cmd_sense(u8 *sense) {

    u16 resp;
    u8 i;
    u8 cmd[6];

    cmd[0] = CD_CMD_SENSE;
    cmd[1] = 0x00;
    cmd[2] = 0x00;
    cmd[3] = 0x00;
    cmd[4] = 0x0A;
    cmd[5] = 0x00;

    resp = cd_tx_cmd(cmd, sizeof (cmd));
    if (resp)return resp;

    for (i = 0; i < 10; i++) {
        *sense++ = cd_dat_rd();
    }

    return cd_read_resp();
}

u8 *cd_rd_ptr;
u8 cd_rd_len;

void cd_tx_sectors(void *dst, u8 num) {

    cd_rd_len = num;
    cd_rd_ptr = dst;
    cd_tx_sectors_asm();
}

void cd_tx_sectors_sw(void *dst, u8 num) {

    cd_rd_len = num;
    cd_rd_ptr = dst;
    cd_tx_sectors_asm_sw();
}

#include "main.h"

void tst_cd_ops() {

    u16 resp;
    u8 toc[4];
    u16 i, u;

    gConsPrint("cd init...");
    cd_init();
    gAppendString("ok");


    for (u = 1; u < 8; u++) {

        u16 toc_cmd = u; // << 8;
        toc_cmd |= 0x0200;

        gConsPrint("toc:      ");
        resp = cd_cmd_toc(toc, toc_cmd);
        for (i = 0; i < 4; i++)gAppendHex8(toc[i]);
        gAppendString(":");
        gAppendHex16(resp);
    }

    gConsPrint("play d8...");
    resp = cd_cmd_play_D8(0x083266, PMOD_D8_SEEK, AMOD_MSF);
    gAppendHex16(resp);
    for (i = 0; i < 32; i++) {
        gVsync();
    }
    //status_monitor(32);

    gConsPrint("pause.....");
    //sysJoyWait();
    i = cd_cmd_pause();
    gAppendHex16(i);
    for (i = 0; i < 32; i++) {
        gVsync();
    }

    gConsPrint("play d9...");
    //sysJoyWait();
    resp = cd_cmd_play_D9(0x083866, PMOD_D9_PE, AMOD_MSF); //+6 sec
    gAppendHex16(resp);
    for (i = 0; i < 60; i++) {
        gVsync();
    }

    gConsPrint("read......");
    //sysJoyWait();
    resp = cd_cmd_read_mem(cbuff, 0x0e06, 2);
    gAppendHex16(resp);


    gConsPrint("");
    for (i = 0; i < 16; i++) {
        u8 resp;
        resp = cbuff[i];
        gAppendHex8(resp);
    }

    gConsPrint("");
    for (i = 0; i < 16; i++) {
        u8 resp;
        resp = cbuff[i + 2048 + 32];
        gAppendHex8(resp);
    }


    gConsPrint("end");

}


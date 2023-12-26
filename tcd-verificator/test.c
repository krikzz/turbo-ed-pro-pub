
#include "main.h"

u8 err_ctr;

void test_all() {

    u8 resp;
    u8 bank = sysGetBank();

    gCleanScreen();
    err_ctr = 0;
    //ad_reset();

    gConsPrint("registers r/w");
    resp = tst_regs_rw();
    printResp(resp);

    gConsPrint("reg 1803:1");
    resp = tst_regs_1803_1();
    printResp(resp);

    gConsPrint("bram 2k");
    resp = tst_bram2k();
    printResp(resp);

    gConsPrint("work ram 64K");
    resp = tst_wram();
    printResp(resp);


    gConsPrint("adpcm ram");
    resp = tst_ad_ram();
    printResp(resp);

    gConsPrint("adpcm reset");
    resp = tst_ad_reset();
    printResp(resp);

    /*
    gConsPrint("adpcm ram mode");
    resp = tst_ad_ram_mode();
    printResp(resp);*/

    gConsPrint("adpcm len ctr");
    resp = tst_ad_len_ctr();
    printResp(resp);

    gConsPrint("adpcm play");
    resp = tst_ad_play();
    printResp(resp);

    gConsPrint("adpcm irq");
    resp = tst_ad_irq(0);
    printResp(resp);



    resp = tst_cdd();
    if (resp) {
        gConsPrint("cd test aborted");
        printResp(resp);
    }

    gSetPal(PAL_01);
    gConsPrint("");
    gConsPrint("errors count ");
    if (err_ctr == 0) {
        gSetPal(PAL_OK);
    } else {
        gSetPal(PAL_ER);
    }

    gSetX(17);
    gAppendNum(err_ctr);
    //gSetPal(PAL_01);
    gSetX(0);
    gConsPrint("END");

    cd_reset();
    sysJoyWait();

    sysSetBank(bank);
    
    return;
}

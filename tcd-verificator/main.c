
#include "main.h"

u8 cbuff[4096];
u8 irq_ctr;

u8 tst_cdd_rd_dma2();

void tst() {

    u8 *arc = (u8 *) 0x1A00;

    gConsPrint("arc: ");
    gAppendHex8(arc[0xfe]);
    gConsPrint("arc: ");
    gAppendHex8(arc[0xff]);

    arc[9] = 0x11;
    
    /*
    arc[7 + 16] = 1;
    arc[7 + 16 * 2] = 1;
    arc[7 + 16 * 3] = 1;*/

    arc[7] = 1;

    arc[2] = 0;
    arc[3] = 0;
    arc[4] = 0;


    arc[0] = 0x12;
    arc[1] = 0x34;

    arc[2] = 0;
    arc[3] = 0;
    arc[4] = 0;



    gPrintHex(arc, 256);

    gConsPrint("");
    sysSetBank(0x40);
    gPrintHex((u8 *) ADDR_RAM_BANK, 16);

    sysJoyWait();
}

void main() {

    sysInit();

    /*
    cbuff[0] = tst_cdd_cmd_time();
    gConsPrint("resp: ");
    gAppendHex8(cbuff[0]);
    sysJoyWait();*/

    //tst_print_1803();
    //tst33();
    //tst();

    menu();
    gCleanScreen();
    gConsPrint("why i'm here?");

    while (1);
}

void printResp(u8 code) {


    gSetX(17);

    if (code == 0) {
        gSetPal(PAL_OK);
        gAppendString("OK");
    } else {

        gSetPal(PAL_ER);
        gAppendString("ER.");
        gAppendHex8(code);
        err_ctr++;
    }

    gSetPal(PAL_00);
    gSetX(0);
}

u16 get_ticks() {//everdrive timer

    REG_TIMER = 0;
    asm("nop");
    return REG_TIMER;
}
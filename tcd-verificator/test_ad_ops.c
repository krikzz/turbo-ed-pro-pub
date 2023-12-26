
#include "main.h"

void tst_ad_rw() {

    u16 i;
    u8 size = 128;

    for (i = 0; i < size; i++) {
        cbuff[i] = i;
    }

    ad_reset();
    ad_ram_wr(cbuff, 0, size);
    ad_ram_rd(cbuff, 0, size);

    gConsPrint("");
    gPrintHex(cbuff, size);
}

void tst_ad_play_sample() {

    u16 i;

    ad_reset();
    ad_set_addr_wr(0);

    for (i = 0; i < (sizeof (sample)); i++) {

        u8 sval = sample[i];
        ad_byte_wr(sval);
    }

    ad_play_start(0, sizeof (sample), 0);

    gConsPrint("done!");
    while (1);
}


#include "main.h"
#include "app_map.h"

void menu_app();

void menu() {

    u8 bank = sysGetBank();
    sysSetBank(APP_MENU);
    menu_app();
    sysSetBank(bank);
}

#pragma codeseg ("BNK01")

void menuInitCdd();
void menuLoadSample();
void menuPrintToc();
void menuPlayCdda();
void menuPrintSubq();
void menuCddaTimings();
void menuCdLongRead();

enum {
    MENU_TEST_ALL,
    MENU_INIT_CDD,
    MENU_LOD_SAMPLE,
    MENU_PLAY_SAMPLE_1,
    MENU_PLAY_SAMPLE_X,
    MENU_STOP_SAMPLE,
    MENU_PRINT_TOC,
    MENU_PLAY_CDDA,
    MENU_CDDA_TIMINGS,
    MENU_CD_RD,
    MENU_SIZE
};

void menu_app() {

    u8 selector;
    u8 i;
    u8 joy;
    static u8 * menu_str[MENU_SIZE + 1];

    menu_str[MENU_TEST_ALL] = "test all";
    menu_str[MENU_INIT_CDD] = "init cdd";
    menu_str[MENU_LOD_SAMPLE] = "load sample";
    menu_str[MENU_PLAY_SAMPLE_1] = "play sample once";
    menu_str[MENU_PLAY_SAMPLE_X] = "play sample forever";
    menu_str[MENU_STOP_SAMPLE] = "stop sample";
    menu_str[MENU_PRINT_TOC] = "print toc";
    menu_str[MENU_PLAY_CDDA] = "play cdda";
    menu_str[MENU_CDDA_TIMINGS] = "cdda timings";
    menu_str[MENU_CD_RD] = "cd long read";

    selector = 0;

    //ad_byte_rd(0);
    while (1) {

        gSetXY(0, (G_SCREEN_H - MENU_SIZE * 1) / 2 - 2);

        for (i = 0; i < MENU_SIZE; i++) {
            if (selector == i) {
                gSetPal(PAL_01);
            } else {
                gSetPal(PAL_00);
            }
            gConsPrintCX(menu_str[i]);
            //gConsPrintCX("");

            gSetPal(PAL_00);
        }

        joy = sysJoyWait();

        if (joy == JOY_U) {
            selector = selector == 0 ? MENU_SIZE - 1 : selector - 1;
        }

        if (joy == JOY_D) {
            selector = selector == MENU_SIZE - 1 ? 0 : selector + 1;
        }

        if (joy != JOY_B)continue;

        switch (selector) {
            case MENU_TEST_ALL:
                test_all();
                break;
            case MENU_INIT_CDD:
                menuInitCdd();
                break;
            case MENU_LOD_SAMPLE:
                menuLoadSample();
                break;
            case MENU_PLAY_SAMPLE_1:
                ad_play_stop();
                ad_play_start(0, sizeof (sample), 0);
                break;
            case MENU_PLAY_SAMPLE_X:
                ad_play_stop();
                ad_play_start(0, sizeof (sample), 1);
                break;
            case MENU_STOP_SAMPLE:
                ad_play_stop();
                break;
            case MENU_PRINT_TOC:
                menuPrintToc();
                break;
            case MENU_PLAY_CDDA:
                menuPlayCdda();
                break;
            case MENU_CDDA_TIMINGS:
                menuCddaTimings();
                break;
            case MENU_CD_RD:
                menuCdLongRead();
                break;
        }
        gCleanScreen();
    }

}

void menuInitCdd() {

    gCleanScreen();
    gConsPrint("init cdd...");
    cd_init();
    gAppendString("ok");
    gConsPrint("press any key");
    sysJoyWait();
}

void menuLoadSample() {

    ad_reset();
    gCleanScreen();
    gConsPrint("loading...");
    ad_play_stop();
    ad_ram_wr(sample, 0, sizeof (sample));
    ad_ram_wr(sample, sizeof (sample), sizeof (sample));
    gAppendString("ok");
}

void menuPrintToc() {

    u8 i, u;
    u8 min;
    u8 max;
    u8 toc[4];
    u16 resp = 0;
    u32 crc = 0;
    gCleanScreen();

    gConsPrint("read toc...");

    resp = cd_cmd_toc(toc, 0x0000);
    crc = crc32(crc, toc, 4);
    min = toc[0];
    max = toc[1];

    if (resp == 0) {
        resp = cd_cmd_toc(toc, 0x0100);
        crc = crc32(crc, toc, 4);
    }

    gConsPrint("min: ");
    gAppendHex8(min);
    gAppendString(", max: ");
    gAppendHex8(max);
    gAppendString(", siz: ");
    for (i = 0; i < 4; i++) {
        gAppendHex8(toc[i]);
    }

    gConsPrint("");

    if (max > 0x60) {
        max = 0x60;
    }
    i = 0;
    while (resp == 0 && min <= max) {

        if (i % 3 == 0)gConsPrint("");

        gAppendHex8(min);
        gAppendString(":");
        resp = cd_cmd_toc(toc, 0x0200 | min);
        crc = crc32(crc, toc, 4);
        for (u = 0; u < sizeof (toc); u++) {
            gAppendHex8(toc[u]);
        }
        gAppendString(" ");



        //inc bcd value
        if ((min & 0xf) >= 9) {
            min &= 0xf0;
            min += 0x10;
        } else {
            min++;
        }

        i++;
    }

    gConsPrint("");
    gConsPrint("resp:");
    gAppendHex16(resp);
    gAppendString(" crc:");
    gAppendHex32(crc);
    gAppendString(" press any key");
    sysJoyWait();
}

void menuPlayCdda() {

    enum {
        PLAY_D8_40,
        PLAY_D8_01,
        PLAY_D8_41,
        PLAY_D8_81,
        PLAY_DA_00,
        PLAY_D9_40,
        PLAY_D9_41,
        PLAY_D9_42,
        PLAY_D9_43,

        PLAY_SIZE
    };

    u8 * menu[PLAY_SIZE];
    u8 i;
    u8 selector = 0;
    u8 joy;
    u16 resp;
    u8 cscreen = 1;
    u8 track = 1;



    menu[PLAY_D8_40] = "play D8-40 (seek-pause msf)";
    menu[PLAY_D8_01] = "play D8-01 (seek-play lba) ";
    menu[PLAY_D8_41] = "play D8-41 (seek-play msf) ";
    menu[PLAY_D8_81] = "play D8-81 (seek-play tno) ";
    menu[PLAY_DA_00] = "play DA-00 (pause)         ";
    menu[PLAY_D9_40] = "play D9-40 (mute msf)      ";
    menu[PLAY_D9_41] = "play D9-41 (play-loop msf) ";
    menu[PLAY_D9_42] = "play D9-42 (play-end-l msf)";
    menu[PLAY_D9_43] = "play D9-43 (play-end msf)  ";

    while (1) {

        if (cscreen) {
            gCleanScreen();
            cscreen = 0;
        }


        //gSetXY((G_SCREEN_W - 10) / 2, 0);

        gSetXY(0, 0);
        gConsPrint("resp : ");
        gAppendHex16(resp);
        gConsPrint("track: ");
        gAppendHex8(track);

        gSetXY(0, (G_SCREEN_H - PLAY_SIZE * 1) / 2 - 2);

        for (i = 0; i < PLAY_SIZE; i++) {
            if (selector == i) {
                gSetPal(PAL_01);
            } else {
                gSetPal(PAL_00);
            }
            gConsPrintCX(menu[i]);
            //gConsPrintCX("");

            gSetPal(PAL_00);
        }


        //joy = sysJoyWait();
        while (sysJoyRead() != 0) {
            gVsync();
            menuPrintSubq();
        }
        joy = 0;
        while (joy == 0) {
            gVsync();
            joy = sysJoyRead();
            menuPrintSubq();
        }

        if (joy == JOY_L && track > 0) {
            track--;
        }
        if (joy == JOY_R && track < 0x99) {
            track++;
        }

        if (joy == JOY_U) {
            selector = selector == 0 ? PLAY_SIZE - 1 : selector - 1;
        }

        if (joy == JOY_D) {
            selector = selector == PLAY_SIZE - 1 ? 0 : selector + 1;
        }

        if (joy == JOY_A)return;
        if (joy != JOY_B)continue;

        if (selector == PLAY_D9_41 || selector == PLAY_D9_42) {
            gCleanScreen();
            gSetY(G_SCREEN_H / 2);
            gConsPrintCX("play...");
            cscreen = 1;
        }

        switch (selector) {
            case PLAY_D8_40:
                resp = cd_cmd_play_D8(SECTOR_CDDA_S, PMOD_D8_SEEK, AMOD_MSF);
                break;
            case PLAY_D8_01:
                resp = cd_cmd_play_D8(SECTOR_LBA, PMOD_D8_PLAY, AMOD_LBA);
                break;
            case PLAY_D8_41:
                resp = cd_cmd_play_D8(SECTOR_CDDA_S, PMOD_D8_PLAY, AMOD_MSF);
                break;
            case PLAY_D8_81:
                resp = cd_cmd_play_D8(track, PMOD_D8_PLAY, AMOD_TRA);
                break;
            case PLAY_DA_00:
                resp = cd_cmd_pause();
                break;
            case PLAY_D9_40:
                resp = cd_cmd_play_D9(SECTOR_CDDA_E, PMOD_D9_PM, AMOD_MSF);
                break;
            case PLAY_D9_41:
                resp = cd_cmd_play_D9(SECTOR_CDDA_E, PMOD_D9_PL, AMOD_MSF);
                break;
            case PLAY_D9_42:
                resp = cd_cmd_play_D9(SECTOR_CDDA_E, PMOD_D9_PE_LC, AMOD_MSF);
                break;
            case PLAY_D9_43:
                resp = cd_cmd_play_D9(SECTOR_CDDA_E, PMOD_D9_PE, AMOD_MSF);
                break;
        }

    }
}

void menuPrintSubq() {

    gSetXY(0, 20);
    cd_print_subq();
}

void menuCddaTimings() {

    u16 i;
    u16 time;
    u16 resp;
    Subq sub;
    u32 min, max;


    gCleanScreen();
    cd_init();

    gConsPrint("play1...");
    resp = cd_cmd_play_D8(0x083811, PMOD_D8_PLAY, AMOD_MSF);
    time = get_ticks();
    while ((get_ticks() - time) < 2154);
    resp = cd_cmd_pause();

    for (i = 0; i < 30; i++)gVsync();

    gConsPrint("play2...");
    resp = cd_cmd_play_D8(0x083811, PMOD_D8_SEEK, AMOD_MSF);
    resp = cd_cmd_play_D9(0x084011, PMOD_D9_PE, AMOD_MSF);
    time = get_ticks();

    while (1) {
        cd_cmd_read_subq(&sub);
        if (sub.play_stat == 3)break;
    }


    time = get_ticks() - time;
    gConsPrint("time: ");
    gAppendNum(time);

    for (i = 0; i < 30; i++)gVsync();

    gConsPrint("play3...");
    cd_cmd_play_D8(0x083811, PMOD_D8_PLAY, AMOD_MSF);
    time = get_ticks();

    while (1) {

        u32 addr = 0;
        cd_cmd_read_subq(&sub);
        addr = ((u32) sub.msf_in_disk[0] << 16) | ((u16) sub.msf_in_disk[1] << 8) | sub.msf_in_disk[2];

        if (addr >= 0x084011)break;
    }
    time = get_ticks() - time;
    cd_cmd_pause();

    gConsPrint("time: ");
    gAppendNum(time);

    resp = cd_cmd_play_D8(0x083811, PMOD_D8_SEEK, AMOD_MSF);

    min = 0xffffff;
    max = 0;
    for (i = 0; i < 100; i++) {
        u32 addr = 0;
        cd_cmd_read_subq(&sub);
        addr = ((u32) sub.msf_in_disk[0] << 16) | ((u16) sub.msf_in_disk[1] << 8) | sub.msf_in_disk[2];

        if (min > addr)min = addr;
        if (max < addr)max = addr;
    }

    gConsPrint("min: ");
    gAppendHex32(min);
    gConsPrint("max: ");
    gAppendHex32(max);


    gConsPrint("seek: ");
    resp = cd_cmd_play_D8(0x200000, PMOD_D8_SEEK, AMOD_MSF);
    for (i = 0; i < 30; i++)gVsync();
    time = get_ticks();
    resp = cd_cmd_play_D8(0x200500, PMOD_D8_SEEK, AMOD_MSF);
    time = get_ticks() - time;
    gAppendNum(time);
    gAppendChar('.');

    resp = cd_cmd_play_D8(0x200000, PMOD_D8_SEEK, AMOD_MSF);
    for (i = 0; i < 30; i++)gVsync();
    time = get_ticks();
    resp = cd_cmd_play_D8(0x201000, PMOD_D8_SEEK, AMOD_MSF);
    time = get_ticks() - time;
    gAppendNum(time);
    gAppendChar('.');

    resp = cd_cmd_play_D8(0x200000, PMOD_D8_SEEK, AMOD_MSF);
    for (i = 0; i < 30; i++)gVsync();
    time = get_ticks();
    resp = cd_cmd_play_D8(0x203000, PMOD_D8_SEEK, AMOD_MSF);
    time = get_ticks() - time;
    gAppendNum(time);
    gAppendChar('.');


    resp = cd_cmd_play_D8(0x200000, PMOD_D8_SEEK, AMOD_MSF);
    for (i = 0; i < 30; i++)gVsync();
    time = get_ticks();
    resp = cd_cmd_play_D8(0x210000, PMOD_D8_SEEK, AMOD_MSF);
    time = get_ticks() - time;
    gAppendNum(time);
    gAppendChar('.');

    resp = cd_cmd_play_D8(0x200000, PMOD_D8_SEEK, AMOD_MSF);
    for (i = 0; i < 30; i++)gVsync();
    time = get_ticks();
    resp = cd_cmd_play_D8(0x220000, PMOD_D8_SEEK, AMOD_MSF);
    time = get_ticks() - time;
    gAppendNum(time);
    gAppendChar('.');

    resp = cd_cmd_play_D8(0x200000, PMOD_D8_SEEK, AMOD_MSF);
    for (i = 0; i < 30; i++)gVsync();
    time = get_ticks();
    resp = cd_cmd_play_D8(0x300000, PMOD_D8_SEEK, AMOD_MSF);
    time = get_ticks() - time;
    gAppendNum(time);
    gAppendChar('.');

    menuPrintSubq();

    while (1) {
        menuPrintSubq();
        if (sysJoyRead() != 0)break;
    }
}

u8 tst_cdd_rd_cpu();

void menuCdLongRead() {

    u8 resp;
    u32 err_ctr = 0;
    u32 itr_ctr = 0;

    gCleanScreen();
    gConsPrint("init cdd...");
    resp = cd_init();
    if (resp) {
        gAppendString("init error");
        sysJoyWait();
        return;
    } else {
        gAppendString("ok");
    }

    while (1) {

        gSetXY(0, 4);
        gConsPrint("itr ctr: ");
        gAppendNum(itr_ctr++);
        gConsPrint("err ctr: ");
        gAppendNum(err_ctr);

        resp = tst_cdd_rd_cpu();

        if (resp) {
            gConsPrint("last err: ");
            gAppendHex8(resp);
            err_ctr++;
        }
    }

}
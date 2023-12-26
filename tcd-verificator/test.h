/* 
 * File:   test.h
 * Author: igor
 *
 * Created on September 2, 2022, 12:21 PM
 */

#ifndef TEST_H
#define	TEST_H



void test_all();

u8 tst_bram2k();
u8 tst_wram();
u8 tst_ad_ram();
u8 tst_ad_play();
u8 tst_ad_irq(u8 init);
u8 tst_ad_ram_mode();
u8 tst_regs_rw();
u8 tst_regs_1803_1();

void tst_cd_ops();
void tst_ad_rw();
void tst_ad_play_sample();
u8 tst_ad_len_ctr();
u8 tst_ad_reset();

u8 tst_cdd();
u8 tst_cdd_cp();

extern u8 err_ctr;

#endif	/* TEST_H */


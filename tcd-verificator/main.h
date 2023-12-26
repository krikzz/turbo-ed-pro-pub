/* 
 * File:   main.h
 * Author: igor
 *
 * Created on August 9, 2022, 11:34 PM
 */

#ifndef MAIN_H
#define	MAIN_H

#include "cfg.h"
#include "sys.h"
#include "cd.h"
#include "test.h"
#include "adpcm.h"
#include "crc.h"
#include "menu.h"

#define REG_TIMER       *((u16 *)0x1FF4)

#define SECTOR_CRC1     0x2124FE50
#define SECTOR_CRC2     0x9A2A492B
#define SECTOR_CRC8K    0xAE4492F0
#define SECTOR_CRC64K   0x26F1A713
#define SECTOR_DATA     0x0e06
#define SECTOR_CDDA_S   0x083266
#define SECTOR_CDDA_E   0x083866
#define SECTOR_LBA      ((8*60+34 + 2) * 75 + 66)/256 //cdda lba addr (track 6)
#define SECTOR_VAL0     0x82
#define SECTOR_VAL1     0xb1

void printResp(u8 code);
u16 get_ticks();

extern u8 cbuff[4096];
extern u8 sample[4072];
extern u8 irq_ctr;

#endif	/* MAIN_H */


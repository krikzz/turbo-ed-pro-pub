/* 
 * File:   tcd.h
 * Author: igor
 *
 * Created on September 2, 2022, 12:40 PM
 */

#ifndef TCD_H
#define	TCD_H

#define CD_REG_1800    *((vu8  *)0x1800)//cdc status
#define CD_REG_1801    *((vu8  *)0x1801)//CDC command / status / data
#define CD_REG_1802    *((vu8  *)0x1802)//ADPCM / CD control
#define CD_REG_1803    *((vu8  *)0x1803)//BRAM lock / CD status
#define CD_REG_1804    *((vu8  *)0x1804)//CD reset
#define CD_REG_1805    *((vu8  *)0x1805)//Convert PCM data / PCM data
#define CD_REG_1806    *((vu8  *)0x1806)//Convert PCM data / PCM data
#define CD_REG_1805_16 *((vu16 *)0x1805)//PCM data
#define CD_REG_1807    *((vu8  *)0x1807)//BRAM unlock / CD status
#define CD_REG_1808    *((vu8  *)0x1808)// ADPCM addr lo / CD data
#define CD_REG_1809    *((vu8  *)0x1809)// ADPCM addr lo hi
#define CD_REG_1808_16 *((vu16 *)0x1808)//ADPCM address (LSB)
#define CD_REG_180A    *((vu8  *)0x180A)//ADPCM RAM data port
#define CD_REG_180B    *((vu8  *)0x180B)//ADPCM DMA control
#define CD_REG_180C    *((vu8  *)0x180C)//ADPCM status
#define CD_REG_180D    *((vu8  *)0x180D)//ADPCM address control
#define CD_REG_180E    *((vu8  *)0x180E)//ADPCM playback rate
#define CD_REG_180F    *((vu8  *)0x180F)//ADPCM and CD audio fade timer


#define CD_CMD_TEST     0x00
#define CD_CMD_SENSE    0x03
#define CD_CMD_READ     0x08
#define CD_CMD_SEEK     0xD8
#define CD_CMD_PLAY     0xD9
#define CD_CMD_PAUSE    0xDA
#define CD_CMD_RDSUB    0xDD
#define CD_CMD_TOC      0xDE

#define TOC_TRACKS      0x0000 //min-max track num
#define TOC_DISK_SIZE   0x0100
#define TOC_TRACK_INF   0x0200

#define TCD_BANK_BRAM           0xF7
#define TCD_BANK_WRAM           0x80

#define PMOD_D8_SEEK    0
#define PMOD_D8_PLAY    1

#define PMOD_D9_PM      0//play mute
#define PMOD_D9_PL      1//loop till next cmd will be executed
#define PMOD_D9_PE_LC   2//play to end point. cmd resp after end point
#define PMOD_D9_PE      3//play to end point

#define AMOD_LBA        0x00 //(256 sectors/frames per lba)
#define AMOD_MSF        0x40
#define AMOD_TRA        0x80

#define CD_ERR_CMD_ABORT        0x0080
#define CD_ERR_UNXP_STAT        0x0040

//play_stat:
//$00 PLAYING
//$01 STILL     pause after cmd D9 ?
//$02 PAUSE
//$03 NOT PLAYING


//ctrl:
//00x0 2 AUDIO CHANNELS WITHOUT PRE-EMPP~SIS
//00x1 2 AUDIO CHANNELS WITH PP3-EMPHASIS
//10x0 4 AUDIO CHANNELS WITHOUT PRE-EMPHASIS
//10x1 4 AUDIO CHANNELS WITH PRE-EMPHASIS
//01x0 DATA TRACK
//01x1 //RESERVED
//11x0 //RESERVED
//xx0x //DIGITAL COPY PROHIBITED
//xx1x //DIGITAL COPY PERMITTED

//0x21 for audio, 0x41 for data

//index:
//INDEX BCD (WTF?) 
//always 1, but become 0 in begin of next track

typedef struct {
    
    u8 play_stat;
    u8 ctrl;
    u8 track_num;
    u8 index;
    u8 msf_in_track[3];
    u8 msf_in_disk[3];
} Subq;


void cd_reg_w(u16 reg, u8 val);
u8 cd_reg_r(u16 reg);
u8 cd_status();
void cd_dat_wr(u8 val);
u8 cd_dat_busy();
u8 cd_dat_rd();
u16 cd_tx_cmd(u8 *cmd, u8 len);
u16 cd_read_resp();
void cd_start_cmd();
u16 cd_cmd_00();
u16 cd_cmd_play_D8(u32 addr, u8 pmode, u8 amod);
u16 cd_cmd_play_D9(u32 addr, u8 pmode, u8 amod);
void cd_reset();
u16 cd_init();
u16 cd_cmd_read(u32 lba, u8 blocks);
u16 cd_cmd_read_mem(u8 *dst, u32 lba, u8 blocks);
u16 cd_cmd_read_mem_1801(u8 *dst, u32 lba, u8 blocks);
u16 cd_cmd_read_dma(u32 ad_addr, u32 lba, u8 blocks);
u16 cd_cmd_pause();
u16 cd_cmd_toc(void *toc, u16 arg);
u16 cd_cmd_read_subq(Subq *subq);
u16 cd_print_subq();
u16 cd_cmd_sense(u8 *sense);
void cd_tx_sectors(void *dst, u8 num);
void cd_tx_sectors_sw(void *dst, u8 num);//tx via 1801 instead of 1808

u8 rd_1803();
u8 rd_180C();
#endif	/* TCD_H */


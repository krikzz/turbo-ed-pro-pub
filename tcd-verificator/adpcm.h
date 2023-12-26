/* 
 * File:   adpcm.h
 * Author: igor
 *
 * Created on September 7, 2022, 2:36 AM
 */

#ifndef ADPCM_H
#define	ADPCM_H

void ad_reset();
void ad_set_addr_rd(u16 addr);
void ad_set_addr_wr(u16 addr);
void ad_play_busy();
u8 ad_byte_rd();
void ad_byte_wr(u8 val);
void ad_play_start(u16 addr, u16 size, u8 loop);
void ad_set_rate(u8 rate);
void ad_play_stop();
void ad_ram_rd(u8 *dst, u16 addr, u16 len);
void ad_ram_wr(u8 *src, u16 addr, u16 len);
void ad_dma_start(u16 dst);
void ad_set_sample_len(u16 len);
void ad_dma_busy();
u8 ad_play_end();
u8 ad_play_end_32K();

#endif	/* ADPCM_H */



.segment "BNK00"

VDC_REG     = $100
VDC_REG_MRW = $02

cval        = $0120 
clr_row:
.word cval, cval, cval, cval, cval, cval, cval, cval
.word cval, cval, cval, cval, cval, cval, cval, cval
.word cval, cval, cval, cval, cval, cval, cval, cval
.word cval, cval, cval, cval, cval, cval, cval, cval

.export _g_clean_plan
_g_clean_plan:
    ldx #28
    lda #VDC_REG_MRW
    sta VDC_REG
@1:
    tia clr_row, 2,  64
    tia clr_row, 2,  64
  
    dex
    bne @1
    rts

.export _sys_set_bank
.import _sys_bank
_sys_set_bank:
    lda _sys_bank
    tam #4
    rts

.export _font
_font:
.incbin "font.bin"
nop

.segment "CODE"
.export _sample
_sample:
.incbin "sample.raw"
nop


.export _ad_set_rd
_ad_set_rd:
    LDA   #$08
    TSB   $180D
    LDA   $180A ;does not work properly without this weird stuff. addr can be changet only during 180A r/w?
    LDA   #$05
@0:
    DEC 
    BNE   @0
    LDA   #$08
    BRA   ad_set_end

.export _ad_set_wr
_ad_set_wr:
    LDA   #$03  ;set bit 0:1
    TSB   $180D
    LDA   #$01  ;clear bit 0
    TRB   $180D
    LDA   #$02  ;clear bit 1
    BRA   ad_set_end

.export _ad_set_len
_ad_set_len:
    LDA   #$10
    TSB   $180D
    LDA   #$10

ad_set_end:;$1725
    TRB   $180D
    RTS

.export _ad_rst
_ad_rst:
    LDA   #$80
    STA   $180D
    STZ   $180D
    STZ   $180B
    LDA   #$6F
    TRB   $1802
    STZ   $180E
    RTS 

.import _cd_rd_ptr
.import _cd_rd_len
.export _cd_tx_sectors_asm
_cd_tx_sectors_asm:
    lda _cd_rd_ptr+1
    sta $81
    lda _cd_rd_ptr
    sta $80
    ldy #0
@0:
    lda $1800
    cmp #$c8
    bne @0

    ldx #8
@1:
    lda $1808
    sta ($80), y
    iny
    bne @1

    inc $81
    dex
    bne @1

    dec _cd_rd_len
    bne @0

    rts

.export _cd_tx_sectors_asm_sw
_cd_tx_sectors_asm_sw:
    lda _cd_rd_ptr+1
    sta $81
    lda _cd_rd_ptr
    sta $80
    ldy #0
@0:
    lda $1800
    cmp #$c8
    bne @0

    ldx #8
@1:
    lda $1800
    lda $1801
    sta ($80), y

    lda #$80
    tsb $1802
    lda $1800
    lda #$80
    trb $1802
    

    iny
    bne @1

    inc $81
    dex
    bne @1

    dec _cd_rd_len
    bne @0

    rts

.export _ad_wr_tst
_ad_wr_tst:
    ldy #8
    ldx #0
@0:
    lda $1808
    sta $180a
    nop
    nop
    nop
    nop
    dex
    bne @0
    dey
    bne @0
    rts

#pragma once

#ifndef __UNIT_TEST__
  #define EEPROM_SPACE 512 /* bytes */
#endif


#define EEPROM_HDR_LEN 4 /* Mnemonic, payload_length */
#define AQ_HDR_LEN 6 /* Mnemonic, payload_length, CKSUM */

/* packets (16bit) */
#define CHAIN_AQ 0x0001

#define BRICK_CONT 0x0100
#define BRICK_NAME 0x0101
#define BRICK_BC 0x0102
#define BRICK_PREP 0x0103

#define TMTY_BRNR 0x0200
#define TMTY_BAT 0x0201

#define PGM_DATA 0x0300
#define PGM_STAT 0x0301

#define ERR_TX 0xff00
/* ---------------- */

int16_t brick_cont_len_without_prep(uint16_t cont_index);
int16_t nth_subpkt_by_type(uint16_t pkt, uint16_t cont_index, uint8_t n);
int16_t nth_subpkt_by_index(uint16_t cont_index, uint8_t n);
int16_t nth_pkt_by_type(uint16_t pkt, uint16_t f_index, uint16_t len, uint8_t n);
int16_t nth_pkt_by_index(uint16_t f_index, uint16_t len, uint8_t n);

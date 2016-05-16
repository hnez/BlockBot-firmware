#pragma once

enum rc_states {
  RC_S_CLOSED,
  RC_S_SENTSYN,
  RC_S_SENTSYNACK,
  RC_S_OPEN,
  RC_S_SENTFIN,
  RC_S_SENTFINACK
};

enum rc_pkg_state {
  RC_P_FREE,
  RC_P_COLLECTING,
  RC_P_SENT,
  RC_P_ACKED,
  RC_P_RECEIVED
};

#define FLAG_SYN_MASK _BV(7)
#define FLAG_ACK_MASK _BV(6)
#define FLAG_FIN_MASK _BV(5)

#define BUFSLOT_COUNT 5
#define PAYLOAD_MAXLEN 16

struct rs_header_t {
  uint8_t pipe_wds;
  uint8_t seq_num;
  uint8_t ack_num;
  uint8_t flag_len;
};

struct rs_packet_t {
  struct rs_header_t hdr;
  uint8_t pl[PAYLOAD_MAXLEN];
};

struct rs_bufslot_t {
  uint8_t state;

  struct rs_packet_t packet;
};

struct rs_connection_t {
  uint8_t state;
  uint8_t seqnum_local;
  uint8_t seqnum_remote;
  uint8_t datapipe;

  uint16_t keepalive_timer;
  uint16_t timeout_timer;

  struct rs_bufslot_t buf[BUFSLOT_COUNT];
};

typedef struct rs_connection_t rs_connection;

inline uint8_t rs_dec_pipe(rs_packet_t *p)
{
  return ((p->hdr.pipe_wds>>4)&0x0f);
}

inline uint8_t rs_dec_wdsize(rs_packet_t *p)
{
  return ((p->hdr.pipe_wds)&0x0f);
}

inline uint8_t rs_dec_pllen(rs_packet_t *p)
{
  return ((p->hdr.flag_len)&0x1f);
}

inline void rs_set_pllen(rs_packet_t *p, uint8_t len)
{
  p->hdr.flag_len = (p->hdr.flag_len & 0xe0) | (len & 0x1f);
}

inline uint8_t rs_is_syn(rs_packet_t *p)
{
  return ((p->hdr.flag_len & FLAG_SYN_MASK) ? 1 : 0);
}

inline uint8_t rs_is_ack(rs_packet_t *p)
{
  return ((p->hdr.flag_len & FLAG_ACK_MASK) ? 1 : 0);
}

inline uint8_t rs_is_fin(rs_packet_t *p)
{
  return ((p->hdr.flag_len & FLAG_FIN_MASK) ? 1 : 0);
}

inline int8_t rs_get_slot_with_state(rs_connection *c, uint8_t state)
{
  /*
   * Find a buffer slot with state `state`. Returns the slot number or
   * -1 if no free slot could be found.
   */

  for (int8_t i=0; i<BUFSLOT_COUNT; i++) {
    if (c->buf[i].state= state) {
      return (i);
    }
  }

  return (-1);
}

inline int8_t rs_get_free_slot(rs_connection *c)
{
  return (rs_get_slot_with_state(c, RC_P_FREE));
}


inline int8_t rs_count_free_slot(rs_connection *c)
{
  int slots=0;

  for (int8_t i=0; i<BUFSLOT_COUNT; i++) {
    if (c->buf[i].state= RC_P_FREE) {
      slots++;
    }
  }

  return (slots);
}

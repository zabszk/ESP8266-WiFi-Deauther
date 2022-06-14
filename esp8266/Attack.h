#pragma once

#include "Arduino.h"
#include "UART.h"
#include <ESP8266WiFi.h>

extern "C" {
  #include "user_interface.h"
}

class Attack {
  private:
    static uint8_t wifi_channel;
    static uint8_t deauth_frame[];

  public:
    static bool is_aggressive;
    static bool sendPacket(uint8_t* packet, uint16_t packetSize, uint8_t ch, bool force_ch);
    static void setWifiChannel(uint8_t ch, bool force);
    static uint8_t getWifiChannel();
    static void nextChannel();
    static void deauth(byte *sta, byte *ap);
};

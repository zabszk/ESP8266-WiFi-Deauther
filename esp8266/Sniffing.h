#pragma once

#include "Attack.h"
#include "Structs.h"
#include "List.h"
#include "Time.h"

extern "C" {
  #include "user_interface.h"
}

class Sniffing {
  private:
    static uint32_t nextJump;
    static byte *buffer;
    static bool ap_mode;
    static void remove_expired();
    static uint32_t connect_cooldown;
  
  public:
    static bool channel_lock;
    static bool no_ap;
    static bool wait;
    static ap_list *aps;
    static void init();
    static void configure_wifi();
    static void parse_frame(uint8_t* buf, uint16_t len);
    static void tick();
    static bool switch_to_sniffing(bool force);
    static void switch_to_hosting();
};

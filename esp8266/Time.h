#pragma once
#include "Arduino.h"

class Time {
  public:
    static uint32_t currentTime;
    static void tick();
};

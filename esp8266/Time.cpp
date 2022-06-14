#pragma once
#include "Time.h"

uint32_t Time::currentTime = 0;

void Time::tick() {
  currentTime = millis();
}

#define byte uint8_t
#define DEBUG_ESP_PORT Serial

extern "C" {
  #include "user_interface.h"
}

#include "Arduino.h"
#include "UART.h"
#include "Sniffing.h"
#include "Time.h"
#include "WebServer.h"

void setup() {
  UART::init();
  Sniffing::init();
  WebServer::init();
}

void loop() {
  Time::tick();
  UART::tick();
  Sniffing::tick();
  delay(5);
}

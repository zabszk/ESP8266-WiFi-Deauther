#pragma once
#define UARTBufferLength 32

#include "Arduino.h"
#include "Attack.h"
#include "Sniffing.h"

class UART {
  private:
    static char buffer[];
    static byte bufferLen;
    static bool cleared;
    static void process_command();
  
  public:
    static void init();
    static void tick();
    static void clear();
    static void restore();
    static void println(String s);
    static void print_error(String s);
    static void print_warning(String s);
    static void print_success(String s);
};

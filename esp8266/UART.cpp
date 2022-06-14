#pragma once

#include "UART.h"

char UART::buffer[UARTBufferLength]; 
byte UART::bufferLen = 0;
bool UART::cleared = false;

void UART::init() {
  Serial.begin(115200);
}

void UART::tick() {
  if (Serial.available() <= 0)
    return;
  
  int sr = Serial.read();

  if (sr == 27) { //ESC key
    clear();
    memset(buffer, '\0', UARTBufferLength);
    bufferLen = 0;
    cleared = false;
    return;
  }
  
  if (sr == '\n' || sr == '\r' || sr == '\0') {
    if (bufferLen == 0)
      return;

    clear();
    
    Serial.printf("\033[35m>>\033[39m %s\r\n", buffer);
    process_command();
    memset(buffer, '\0', UARTBufferLength);
    bufferLen = 0;
    cleared = false;
    return;
  }

  if (sr != '\b' && (sr < 32 || sr > 126))
    return;

  if (sr == '\b') {
    if (bufferLen == 0)
      return;
      
    bufferLen--;
    buffer[bufferLen] = '\0';
    
    if (cleared)
      return;
      
    Serial.print("\b \b");
    return;
  }

  if (bufferLen >= UARTBufferLength)
    return;

  buffer[bufferLen] = (char)sr;
  bufferLen++;

  if (cleared)
    return;
  
  Serial.print((char)sr);
}

void UART::clear() {
  cleared = true;
  for (char i = 0; i < bufferLen; i++)
    Serial.print("\b \b");
}

void UART::restore() {
  Serial.printf("%s", buffer);
  cleared = false;
}

void UART::println(String s) {
  if (cleared) {
    Serial.println(s);
    return;
  }
  
  clear();
  Serial.println(s);
  restore();
}

void UART::print_error(String s) {
  println("\033[31m" + s + "\033[39m");
}

void UART::print_warning(String s) {
  println("\033[33m" + s + "\033[39m");
}

void UART::print_success(String s) {
  println("\033[32m" + s + "\033[39m");
}

void UART::process_command() {
  String command = "";
  byte args;
  
  for (args = 0; args < UARTBufferLength && buffer[args] != '\0' && buffer[args] != ' '; args++)
    command += buffer[args];

  if (args + 1 < UARTBufferLength)
    args++;

  if (command == "help") {
    print_success("--- HELP ---\r\n");
    Serial.println("- GENERAL -");
    Serial.println("ap - enables WiFi AP to control this device [IN DEVELOPMENT]");
    Serial.println("help - prints this help");
    Serial.println("list - lists WiFi networks");
    Serial.println("noap - disables WiFI AP");
    Serial.println();
    Serial.println("- RADIO -");
    Serial.println("chn - prints current channel");
    Serial.println("lock <channel number> - locks WiFi adapter on a specific channel");
    Serial.println("unlock - unlocks WiFi adapter");
    Serial.println();
    Serial.println("- ATTACK -");
    Serial.println("s <WiFi #> - (de)selects as attack target");
    Serial.println("start - starts the deauthentication attack on selected networks");
    Serial.println("stop - stops the deauthentication attack");
    
    return;
  }

  if (command == "chn") {
    print_success("Current channel: " + String(Attack::getWifiChannel()) + ".");
    return;
  }

  if (command == "noap") {
    if (Sniffing::no_ap) {
      print_warning("No AP mode is already enabled!");
      return;
    }
    
    Sniffing::no_ap = true;
    Sniffing::wait = false;
    print_success("No AP mode has been enabled.");
    return;
  }

  if (command == "start") {
    if (Attack::is_aggressive) {
      print_warning("Attack is already running!");
      return;
    }

    Attack::is_aggressive = true;
    print_success("Deauthentication attack has been started.");
    return;
  }

  if (command == "stop") {
    if (!Attack::is_aggressive) {
      print_warning("Attack is not running!");
      return;
    }

    Attack::is_aggressive = false;
    print_success("Deauthentication attack has been stopped.");
    return;
  }

  if (command == "ap") {
    if (!Sniffing::no_ap) {
      print_warning("No AP mode is already disabled!");
      return;
    }
    
    Sniffing::no_ap = false;
    print_success("No AP mode has been disabled.");
    return;
  }

  if (command == "unlock") {
    if (!Sniffing::channel_lock) {
      print_warning("Channel is not locked!");
      return;
    }
    
    Sniffing::channel_lock = false;
    print_success("Channel unlocked.");
    return;
  }

  if (command == "lock") {
    if (buffer[args] == '\0') {
      print_error("Missing argument!");
      return;
    }

    String p = "";
    p += buffer[args];
    if (buffer[args + 1] != '\0')
      p += buffer[args + 1];

    int ch = p.toInt();

    if (ch <= 0 || ch > 14) {
      println("Invalid port number");
      return;
    }

    Sniffing::channel_lock = true;

    wifi_promiscuous_enable(false);
    Attack::setWifiChannel(ch, true);
    wifi_promiscuous_enable(true);

    Serial.printf("\033[32mChannel set to %i and locked.\033[39m\r\n", ch);
    return;
  }

  if (command == "s") {
    if (buffer[args] == '\0') {
      print_error("Missing argument!");
      return;
    }

    String p = "";
    p += buffer[args];
    if (buffer[args + 1] != '\0') {
      p += buffer[args + 1];

      if (buffer[args + 2] != '\0')
        p += buffer[args + 2];
    }

    int i = p.toInt();

    if (i < 0) {
      println("Invalid number on the list");
      return;
    }

    access_point *ap = List::get_ap_by_index(Sniffing::aps, i);
    if (ap == NULL) {
      print_error("Invalid WiFi #. Run \"list\" command to get the WiFi #.\r\n");
      return;
    }
    
    ap->setSelected(!ap->getSelected());
    command = "list";
  }

  if (command == "list") {
    ap_node *e = Sniffing::aps->head;
    byte i = 0;
    
    print_success("--- LIST OF NETWORKS ---\r\n");
    Serial.printf(" %-3s | %-3s | %-17s | %-32s | %-14s | %-5s\r\n", "#", "CHN", "BSSID", "SSID", "LAST SEEN [ms]", "FLAGS");
    
    while (e != NULL) {
      Serial.printf(" %-3i | %-3i |", i, e->ap->getChannel());

      for (int i = 0; i < 6; i++) {
        if (e->ap->BSSID[i] <= 0x0F)
          Serial.printf(" 0%hhx", e->ap->BSSID[i]);
        else Serial.printf(" %hhx", e->ap->BSSID[i]);
      }

      if (e->ap->SSID == NULL)
        Serial.printf(" | %-32s | ", "(unknown)");
      else Serial.printf(" | %-32s | ", e->ap->SSID);

      Serial.printf("%-14i |", Time::currentTime - e->ap->last_seen);

      if (e->ap->getHidden())
        Serial.print(" H");

      if (e->ap->getLeakingHidden())
        Serial.print(" L");

      if (e->ap->getSelected())
        Serial.print(" S");

      Serial.println();

      i++;
      e = (ap_node*)e->next;
    }

    Serial.println("\r\nFlags: H - Hidden SSID, L - Hidden SSID leaking SSID length, S - Selected for attacks\r\n--- END OF LIST ---");
    return;
  }

  print_error("Unknown command!");
}

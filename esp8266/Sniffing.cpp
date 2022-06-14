#pragma once
#define SSIDMaxLength 32
#define WiFiExpirationTime 60000

#include "Sniffing.h"
#include "WebServer.h"

uint32_t Sniffing::nextJump = 0;
bool Sniffing::channel_lock = false;
bool Sniffing::no_ap = true;
bool Sniffing::wait = false;
ap_list *Sniffing::aps;
byte *Sniffing::buffer;

uint32_t Sniffing::connect_cooldown = 0;

bool Sniffing::ap_mode = false;

void Sniffing::init() {
  buffer = (byte*)malloc(6*sizeof(byte));
  aps = (ap_list*)malloc(sizeof(ap_list));
  List::ap_list_init(aps);
  wifi_set_opmode(STATION_MODE); // Station mode
  wifi_promiscuous_enable(false);
  wifi_set_promiscuous_rx_cb(Sniffing::parse_frame);
  Attack::setWifiChannel(1, true);
  wifi_promiscuous_enable(true);
}

void Sniffing::configure_wifi() {
  WiFi.persistent(false);
  WiFi.disconnect(true);
  wifi_set_opmode(STATION_MODE); // Station mode
  wifi_set_promiscuous_rx_cb(Sniffing::parse_frame);
}

void Sniffing::parse_frame(uint8_t* buf, uint16_t len) {
  if (len < 28)
    return;

  switch (buf[12]) {
    case 0x08: //Data
    {
      if (!Attack::is_aggressive || len < 34)
        return;
      
      memcpy(buffer, buf + 16, 6);
      access_point *ap = List::get_ap_by_bbsid(aps, buffer);

      if (ap == NULL || !ap->getSelected()) {
        memcpy(buffer, buf + 22, 6);
        ap = List::get_ap_by_bbsid(aps, buffer);
        
        if (ap == NULL || !ap->getSelected()) {
          memcpy(buffer, buf + 28, 6);
          access_point *ap = List::get_ap_by_bbsid(aps, buffer);

          if (ap == NULL || !ap->getSelected())
            break;
        }
      }

      Attack::deauth(buf + 16, buf + 22);
      Attack::deauth(buf + 22, buf + 16);

      Attack::deauth(buf + 16, buf + 28);
      Attack::deauth(buf + 28, buf + 16);

      Attack::deauth(buf + 22, buf + 28);
      Attack::deauth(buf + 28, buf + 22);
    }
    break;

    /*case 0x88: //Data
    {
      if (!Attack::is_aggressive)
        return;

        Serial.print("PRE: ");

        for (int i = 0; i < 6; i++)
          Serial.printf(" [%hhx]", buf[25 + i]);

        Serial.printf(" |");

        for (int i = 0; i < 6; i++)
          Serial.printf(" [%hhx]", buf[21 + i]);

        Serial.printf(" |");

        for (int i = 0; i < 6; i++)
          Serial.printf(" [%hhx]", buf[27 + i]);

        Serial.println();
      
      memcpy(buffer, buf + 25, 6);
      access_point *ap = List::get_ap_by_bbsid(aps, buffer);

      if (ap == NULL) {
        memcpy(buffer, buf + 31, 6);
        ap = List::get_ap_by_bbsid(aps, buffer);
        
        if (ap == NULL || !ap->getSelected()) {
          memcpy(buffer, buf + 37, 6);
          access_point *ap = List::get_ap_by_bbsid(aps, buffer);

          if (ap == NULL || !ap->getSelected())
            break;
        }
      }

      Attack::deauth(buf + 25, buf + 31);
      Attack::deauth(buf + 31, buf + 25);

      Attack::deauth(buf + 25, buf + 37);
      Attack::deauth(buf + 37, buf + 25);

      Attack::deauth(buf + 31, buf + 37);
      Attack::deauth(buf + 37, buf + 31);

      
    }
    break;*/
    
    case 0x80: //Beacon
    {
      memcpy(buffer, buf + 22, 6);
      access_point *ap = List::get_ap_by_bbsid(aps, buffer);

      if (buf[49] > SSIDMaxLength)
        buf[49] = SSIDMaxLength;

      if (ap == NULL) {
        ap = (access_point*)malloc(sizeof(access_point));

        ap->flags = 0;
        ap->BSSID = (byte*)malloc(6*sizeof(byte));
        memcpy(ap->BSSID, buffer, 6);

        if (buf[49] == 0 || buf[50] == '\0') {
          ap->SSID = NULL;
          ap->setHidden(true);

          if (buf[49] > 0 && buf[50] == '\0')
            ap->setLeakingHidden(true);
        }
        else {
          ap->SSID = (char*)malloc((buf[49] + 1)*sizeof(char));
          memcpy(ap->SSID, buf + 50, buf[49]);
          (ap->SSID)[buf[49]] = '\0';
        }

        ap->setChannel((byte)Attack::getWifiChannel());
        ap->last_seen = Time::currentTime;
        List::ap_list_append(aps, ap);
        break;
      }
      
      ap->setChannel((byte)Attack::getWifiChannel());
      ap->last_seen = Time::currentTime;
      ap->setHidden(buf[49] == 0 || buf[50] == '\0');
      ap->setLeakingHidden(buf[50] == '\0');
      
      if (!ap->getHidden() && (ap->SSID == NULL || strcmp(ap->SSID, (char*)(buf + 50)) != 0)) {
        if (ap->SSID == NULL)
          ap->SSID = (char*)malloc((buf[49] + 1)*sizeof(char));
        else if (strlen(ap->SSID) != buf[49]) {
          free(ap->SSID);
          ap->SSID = (char*)malloc((buf[49] + 1)*sizeof(char));
        }

        memcpy(ap->SSID, buf + 50, buf[49]);
        (ap->SSID)[buf[49]] = '\0';
      }
    }
      break;

    /*case 0x40: //Probe Request
      Serial.printf("[%i] Probe Request, STA MAC:", Attack::getWifiChannel());
      for (int i = 0; i < 6; i++)
        Serial.printf(" [%hhx]", buf[22 + i]);
      Serial.printf(", SSID: ");
      for (int i = 0; i < buf[37]; i++)
        Serial.printf("%c", buf[38 + i]);
      Serial.println();
      break;*/

    case 0x50: //Probe Response
    {
      memcpy(buffer, buf + 22, 6);
      access_point *ap = List::get_ap_by_bbsid(aps, buffer);

      if (buf[49] > SSIDMaxLength)
        buf[49] = SSIDMaxLength;

      if (ap == NULL) {
        ap = (access_point*)malloc(sizeof(access_point));

        ap->flags = 0;
        ap->BSSID = (byte*)malloc(6*sizeof(byte));
        memcpy(ap->BSSID, buffer, 6);

        if (buf[49] == 0 || buf[50] == '\0') {
          ap->SSID = NULL;
          ap->setHidden(true);

          if (buf[49] > 0 && buf[50] == '\0')
            ap->setLeakingHidden(true);
        }
        else {
          ap->SSID = (char*)malloc((buf[49] + 1)*sizeof(char));
          memcpy(ap->SSID, buf + 50, buf[49]);
          (ap->SSID)[buf[49]] = '\0';
        }

        ap->setChannel((byte)Attack::getWifiChannel());
        ap->last_seen = Time::currentTime;
        List::ap_list_append(aps, ap);
        break;
      }

      ap->setChannel((byte)Attack::getWifiChannel());
      ap->last_seen = Time::currentTime;

      if (buf[49] == 0 || buf[50] == '\0')
        break;
      
      if (ap->SSID == NULL || strcmp(ap->SSID, (char*)(buf + 50)) != 0) {
        if (ap->SSID == NULL)
          ap->SSID = (char*)malloc((buf[49] + 1)*sizeof(char));
        else if (strlen(ap->SSID) != buf[49]) {
          free(ap->SSID);
          ap->SSID = (char*)malloc((buf[49] + 1)*sizeof(char));
        }

        memcpy(ap->SSID, buf + 50, buf[49]);
        (ap->SSID)[buf[49]] = '\0';
      }

      
      /*Serial.printf("[%i] Probe Response, STA MAC:", Attack::getWifiChannel());
      for (int i = 0; i < 6; i++)
        Serial.printf(" [%hhx]", buf[16 + i]);
      Serial.printf(", AP MAC:");
      for (int i = 0; i < 6; i++)
        Serial.printf(" [%hhx]", buf[22 + i]);
      Serial.printf(", SSID: ");
      for (int i = 0; i < buf[49]; i++)
        Serial.printf("%c", buf[50 + i]);
      Serial.println();*/
    }
      break;
  }
}

void Sniffing::tick() {
  if (ap_mode)
    WebServer::tick();
  
  if (Sniffing::nextJump < Time::currentTime) {
    if (!ap_mode && !no_ap) {
      Sniffing::nextJump = Time::currentTime + 1400;
      switch_to_hosting();
      return;
    }

    if (ap_mode && no_ap) {
      switch_to_sniffing(true);
      wifi_promiscuous_enable(false);
      wifi_set_channel(Attack::getWifiChannel());
      wifi_promiscuous_enable(true);
    }
    else if (ap_mode && !switch_to_sniffing(false)) {
      Sniffing::nextJump = Time::currentTime + 100;
      return;
    }
    
    Sniffing::nextJump = Time::currentTime + 500;
    remove_expired();

    if (channel_lock) {
      if (no_ap)
        return;
      
      wifi_promiscuous_enable(false);
      wifi_set_channel(Attack::getWifiChannel());
      wifi_promiscuous_enable(true);
      return;
    }
    
    wifi_promiscuous_enable(false);
    Attack::nextChannel();
    wifi_promiscuous_enable(true);
  }
}

void Sniffing::remove_expired() {
  byte i = 0;
  ap_node *e = Sniffing::aps->head;
  while (e != NULL) {
    if (Time::currentTime - e->ap->last_seen > WiFiExpirationTime) {
      e = (ap_node*)e->next;
      
      List::ap_list_remove_index(aps, i);
      continue;
    }

    i++;
    e = (ap_node*)e->next;
  }
}

bool Sniffing::switch_to_sniffing(bool force) {
  if (!force) {
    if (connect_cooldown == 0) {
      uint8_t clients = WiFi.softAPgetStationNum();
      if (clients == 1) {
        connect_cooldown = Time::currentTime + 5000;
        wait = true;
      }
    }
    
    if (Time::currentTime < connect_cooldown && wait) {
      UART::println("Client has connected, giving them a moment to stabilise...");
      UART::println(String(connect_cooldown - Time::currentTime) + "ms left");
      return false;
    }
    connect_cooldown = 0;
  }
  
  UART::println("Switching to sniffing mode...");
  Sniffing::configure_wifi();
  ap_mode = false;
  return true;
}

void Sniffing::switch_to_hosting() {
  UART::println("Switching to AP mode...");
  ap_mode = true;
  WebServer::configure_wifi();
  WebServer::tick();
}

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "WebServer.h"
#include "Sniffing.h"

auto server = new ESP8266WebServer(80);

IPAddress local_IP(192,168,4,1);
IPAddress subnet(255,255,255,0);

void WebServer::setup_wifi_ap_mode() {
  wifi_promiscuous_enable(false);
  WiFi.softAPConfig(local_IP, local_IP, subnet);
	WiFi.softAP("ESP8266", "haslohaslo", 1, false, 1);
  WebServer::init();
}

void WebServer::index() {
    String response;

	ap_node *e = Sniffing::aps->head;
    
    //print_success("--- LIST OF NETWORKS ---\r\n");
    //Serial.printf(" %-3s | %-3s | %-17s | %-32s | %-14s | %-5s\r\n", "#", "CHN", "BSSID", "SSID", "LAST SEEN [ms]", "FLAGS");
	response += "<h1>Access Points</h1>";
  response += "<table>";
	response += "<tr><th>SSID</th><th>Channel</th><th>Last Seen</th></tr>";
  while (e != NULL) {
		response += "<tr>";

		if (e->ap->SSID == NULL) {
			response += "<td>(unknown)</td>";
		}
		else {
			response += "<td>" + String(e->ap->SSID) + "</td>";
		}

		response += "<td>" + String(e->ap->getChannel()) + "</td>";
		response += "<td>" + String((Time::currentTime - e->ap->last_seen)/1000.0f) + "s</td>";
    e = (ap_node*)e->next;
  }

	response += "</table><script>setInterval(function(){fetch('/conn');},750)</script>";
  server->send(200, "text/html", response);
}

void WebServer::ok() {
  server->send(200, "text/html", "OK");
  Sniffing::wait = false;
}

void WebServer::configure_wifi() {
	setup_wifi_ap_mode();
}

void WebServer::init() {
  server->on("/", index);
  server->on("/conn", ok);
  server->begin();
}

void WebServer::tick() {
	server->handleClient();
	MDNS.update();
}

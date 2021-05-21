#include <Arduino.h>
#include <ArduinoOTA.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <time.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncWiFiManager.h>        
#include <SPIFFSEditor.h>
#include <esp_system.h>
#include <esp_wifi.h>
#include <pins_arduino.h>
#include <dummy.h> 
#include <FS.h>
#include <LITTLEFS.h>
#include <DNSServer.h>
#include <stdlib.h>
#include <pgmspace.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/event_groups.h"
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <Wire.h>
#include <ESP32SSDP.h>
#include <NetBIOS.h>

extern "C" {
#include "lwip/err.h"
#include "lwip/dns.h"
}
#include <config.h>
RTC_DATA_ATTR int bootCount = 0;
RTC_DATA_ATTR int timesleep = 30;
bool sensloaded = false;


uint8_t clid = 0;
uint8_t clidx = 0;
File xxxf; 

struct conf {
    const char* hostName = "ponika";
    const char* password = "josperado";
    const char* http_username = "admin";
    const char* http_password = "admin";
    int wifi = 2; // 2 - STA, 1 - AP
    bool ota  = 1;
    bool tcp  = 1;
    bool display = 1;
    bool lora = 1;
  std::vector<uint16_t>i2c;
} conf;


time_t now() {return time(NULL);}
time_t session = 0;
time_t runtime = 0;
uint16_t energy = 0;
time_t sec = 0;
time_t osec = 0;
TwoWire I2Cone = TwoWire(1);

String udpAddress = "192.168.88.234";
uint16_t srate = 22050;
bool isadc = false;
uint16_t sdelay = 285;

WiFiUDP udp;
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
AsyncEventSource events("/events");

DNSServer dns;

std::vector<String> tasks;
AsyncEventSourceClient * xxclient;
#include <ArduinoJson.h>
#include <functions.h>
#include <sensors.h>
#include <commands.h>
#include <i2c.h>
#include <setup.h>
#include <tasker.h>

void loop() {
osec = sec;
sec = now();
  ArduinoOTA.handle();
//  dns.processNextRequest();
  if (tasks.size()) dotask(true); //else dotask(false);
  if (osec!=sec) {
    if (session<1600000000) session = now(); runtime = now()-session;
  }
  if (sensloaded) mysensors->maintance();
}
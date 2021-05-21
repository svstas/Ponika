#include <WiFiEvents.h>

uint8_t oprogr = 0;

#ifdef BUTTON  
#define ESP_INTR_FLAG_DEFAULT 0
#define BUTTON_PIN 0
SemaphoreHandle_t semaphore = nullptr;
bool pressed = false;
bool xdisplay = false;
void IRAM_ATTR handler(void* arg) {
  xSemaphoreGiveFromISR(semaphore, NULL);
};
void button_task(void* arg) {
  for(;;) {
    if(xSemaphoreTake(semaphore,portMAX_DELAY) == pdTRUE) {
      xdisplay = !xdisplay;
      sendEvent(String(xdisplay));
    } 
  }
}
#endif
void setup(){

  ++bootCount;

  Serial.begin(115200);
  Serial.setDebugOutput(true);

if (bootCount>1) {
  if (bootCount>10) ESP.restart();
  BMESensor.refresh();
  delay(500);
  Serial.println(bootCount);
  Serial.println(BMESensor.temperature);
  Serial.flush();
  esp_sleep_enable_timer_wakeup(timesleep*1000000);
  esp_deep_sleep_start();
    return;
  }

  I2Cone.begin(21, 22);
  pinMode(batPin,INPUT);
  
  BMESensor.begin();  
  delay(100);
  BMESensor.refresh();

  



#ifdef BUTTON  
semaphore = xSemaphoreCreateBinary();
  gpio_pad_select_gpio((gpio_num_t)0);
  gpio_set_direction((gpio_num_t)0, GPIO_MODE_INPUT);
  gpio_set_intr_type((gpio_num_t)0, GPIO_INTR_NEGEDGE);
  xTaskCreate(button_task, "button_task", 4096, NULL, 10, NULL);
  gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
  gpio_isr_handler_add((gpio_num_t)BUTTON_PIN, handler, NULL);
#endif


 //pinMode(LED, OUTPUT);
 //pinMode(4, OUTPUT);
// digitalWrite(4,LOW);

  trace(F("System starting..."));
  LITTLEFS.begin();
  trace(F("FS init"));
  if (loadConfig()) trace(F("Config loaded"));

if (conf.wifi) WiFi.onEvent(WiFiEvent);

if (conf.wifi ==  1) {  
  WiFi.mode(WIFI_AP);
  WiFi.softAP(conf.hostName,conf.password);
  delay(2000);
  WiFi.softAPConfig(ip, ip, netmask);
  trace(F("WiFi Started"));
} else if (conf.wifi ==  2) {
  AsyncWiFiManager wifiManager(&server,&dns);
  wifiManager.setConfigPortalTimeout(240);
  wifiManager.setBreakAfterConfig(true);
//  wifiManager.resetSettings();

  if (!wifiManager.autoConnect(conf.hostName,conf.password)) {
    Serial.println("failed to connect, we should reset as see if it connects");
    delay(3000);
    ESP.restart();
    ESP.restart();
    delay(5000);
  }
}

    configTime(3600*2, 3600, "pool.ntp.org");

if (conf.ota) {

  ArduinoOTA.onStart([]() { events.send("Update Start", "ota"); });
  ArduinoOTA.onEnd([]() { events.send("Update End", "ota"); });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    char p[32];
   
   uint8_t progr = (progress)/(total/100);
    if (oprogr != progr) {
      oprogr = progr;

    sprintf(p, "Progress: %d\n", progr);
    events.send(p,"m");
    }
  });

  ArduinoOTA.onError([](ota_error_t error) {
    if(error == OTA_AUTH_ERROR) events.send("Auth Failed", "ota");
    else if(error == OTA_BEGIN_ERROR) events.send("Begin Failed", "ota");
    else if(error == OTA_CONNECT_ERROR) events.send("Connect Failed", "ota");
    else if(error == OTA_RECEIVE_ERROR) events.send("Recieve Failed", "ota");
    else if(error == OTA_END_ERROR) events.send("End Failed", "ota");
  });
  ArduinoOTA.setHostname(conf.hostName);
  ArduinoOTA.begin();
}



 
  SSDP.setDeviceType("upnp:rootdevice"); 
  SSDP.setName(conf.hostName);
  SSDP.setSchemaURL("description.xml");
  SSDP.setHTTPPort(80);
  SSDP.begin();



MDNS.addService("http","tcp",80);
//MDNS.begin(conf.hostName);  
NBNS.begin(conf.hostName);  

// MODE_AP!!! NEEDED!!!
//  dns.setErrorReplyCode(DNSReplyCode::NoError);
//  dns.start(53, "*", IPAddress(192,168,4,1));
  Serial.println("DNS Started");
  Serial.println(conf.hostName);
  

if (conf.tcp) {
  ws.onEvent(onWsEvent);
  server.addHandler(&ws);
  events.onConnect([](AsyncEventSourceClient *client){
//    client->send("hello!",NULL,millis(),1000);
    xxclient = client;
//    sendEvent("reconnected");
//    digitalWrite(2,LOW);
  });
  server.addHandler(&events);
  server.addHandler(new SPIFFSEditor(LITTLEFS, conf.http_username,conf.http_password));
  server.on("/generate_204", handleRoot);  //Android captive portal. Maybe not needed. Might be handled by notFound handler.
  server.on("/fwlink", handleRoot);  //Microsoft captive portal. Maybe not needed. Might be handled by notFound handler.
  server.on("/time", HTTP_GET, [](AsyncWebServerRequest *request) {request->send(200, "text/plain", String(now()));});
  server.on("/heap", HTTP_GET, [](AsyncWebServerRequest *request){request->send(200, "text/plain", String(ESP.getFreeHeap()));});
  server.on("/realip", HTTP_GET, [](AsyncWebServerRequest *request){request->redirect("http://"+toStringIp(WiFi.localIP())+"/");});
  server.on("/_cmd", HTTP_GET, [](AsyncWebServerRequest *request){if(request->method() == HTTP_GET && request->hasParam("cmd")){
    String cres = doshell(request->getParam("cmd")->value());
    if (!cres.length()) request->send(304); else request->send(200,"text/html",cres);
    cres = String();}});

  server.serveStatic("/", LITTLEFS, "/").setDefaultFile("index.html");

  server.onNotFound([](AsyncWebServerRequest *request){
    handleRoot(request);
//    request->send(404);
  });
  server.onFileUpload([](AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final){
//    if(!index)
//      trace("UploadStart: %s\n", filename.c_str());
//      trace("%s", (const char*)data);
//    if(final)  trace("UploadEnd: %s (%u)\n", filename.c_str(), index+len);
  });
  server.onRequestBody([](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
    if(!index)
      Serial.printf("BodyStart: %u\n", total);
      Serial.printf("%s", (const char*)data);
    if(index + len == total)
      Serial.printf("BodyEnd: %u\n", total);
  });


  server.begin();
  session = now();
  BMESensor.begin();

//  scan_i2c(false);

  }
}

using
namespace std;  
using namespace std::placeholders;





void setup() {


pinMode(1,OUTPUT);
pinMode(3,OUTPUT);
pinMode(15,OUTPUT);
pinMode(13,OUTPUT);
pinMode(12,OUTPUT);
pinMode(14,OUTPUT);

digitalWrite(1,HIGH);
digitalWrite(3,HIGH);
digitalWrite(15,HIGH);
digitalWrite(13,HIGH);
digitalWrite(12,HIGH); 
digitalWrite(14,HIGH);

Serial.begin(115200);
  Serial.setDebugOutput(true);  
  SPIFFS.begin();
  Serial.println("Starting...");
  Wire.begin();
  loadConfig(false);
  delay(1000);
  pixels.begin(conf.pixelPin);
//  pixels.begin();

  xpix(0,0x00FF00);

Serial.print("---");
Serial.println(conf.ssid);

if (String(conf.ssid)!="") {

WiFi.hostname(conf.xhostname);
    WiFi.begin(conf.ssid, conf.password);



if (WiFi.status() != WL_CONNECTED) {  // FIX FOR USING 2.3.0 CORE (only .begin if not connected)
    WiFi.begin(conf.ssid, conf.password);       // connect to the network
}    
int count;

for (count = 0; count < 20 ; count++ )  {
      delay(500);
      int wifi_stat = WiFi.status();  
      Serial.print( count ); Serial.print( " WifiStat: " );
      Serial.println( wifi_stat );
      if ( wifi_stat == WL_CONNECTED ) {
        break;
      }
}

Serial.println("");

  if ( count < 20 ) {
    Serial.println("WiFi connected");
  }else{
    strcpy(conf.ssid, "");
    strcpy(conf.password, "");
    
    reset_wifi_set();
    delay(500);
    ESP.reset();
  }


} else {

  AsyncWiFiManager wifiManager(&server,&dns);
//  wifiManager.autoConnect("Ponika");

   wifiManager.setBreakAfterConfig(true);

  //reset settings - for testing
  //wifiManager.resetSettings();

  if (!wifiManager.autoConnect(conf.xhostname)) {
      pixels.setPixelColor(3, 0xFF00FF);
      pixels.show();
    Serial.println("failed to connect, we should reset as see if it connects");
    delay(2000);
    ESP.reset();
    delay(3000);
  }

}

  Serial.println("local ip");
  Serial.println(WiFi.localIP());
    
  Serial.print("WiFi Start ");

  if (String(conf.ssid)=="") {
    strcpy(conf.ssid,String(WiFi.SSID()).c_str());
    strcpy(conf.password,String(WiFi.psk()).c_str());
    reset_wifi_set();
  }

  ArduinoOTA.onStart([]() { sendEvent("Update Start", "sms");xpix(0,conf.clr[3]); });
  ArduinoOTA.onEnd([]() { sendEvent("Update End", "sms");xpix(0,conf.clr[0]);});
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    char p[32];
//    xpix(0,conf.clr[3]);
    sprintf(p, "Progress: %u%%\n", (progress/(total/100)));
    sendEvent(p, "sms");
//    events.send(p, "ota");
//  if ((progress/(total/100))==100) xpix(0,conf.clr[0]);
  });
  ArduinoOTA.onError([](ota_error_t error) {
    if(error == OTA_AUTH_ERROR) sendEvent("Auth Failed", "sms");
    else if(error == OTA_BEGIN_ERROR) sendEvent("Begin Failed", "sms");
    else if(error == OTA_CONNECT_ERROR) sendEvent("Connect Failed", "sms");
    else if(error == OTA_RECEIVE_ERROR) sendEvent("Recieve Failed", "sms");
    else if(error == OTA_END_ERROR) sendEvent("End Failed", "sms");
  });
  
  ArduinoOTA.setHostname(conf.xhostname);
  //ArduinoOTA.setPassword("josper99");
  
  ArduinoOTA.begin();
  
  MDNS.addService("http","tcp",80);
  NBNS.begin(conf.xhostname);  
  
  Serial.print("NetBios Start ");
  Serial.println(conf.xhostname);

  
  
  ws.onEvent(onWsEvent);
  server.addHandler(&ws);
  events.onConnect([](AsyncEventSourceClient *client){          
    client->send("hello!",NULL,millis(),1000);
    mysensors->loadNames(true);
    cron.sendR(true);
  });

  server.addHandler(&events);
  server.addHandler(new SPIFFSEditor(conf.http_username,conf.http_password));
  server.on("/heap", HTTP_GET, [](AsyncWebServerRequest *request){request->send(200, "text/plain", String(ESP.getFreeHeap()));});
  server.on("/ip", HTTP_GET, [](AsyncWebServerRequest *request){String out="<a href='http://"+String(WiFi.localIP().toString())+"'>GET BY IP_ADDRESS</a>";request->send(200, "text/html", out); out=String();});
  server.on("/sensors", HTTP_GET, [](AsyncWebServerRequest *request){request->send(200, "text/plain", mysensors->loadNames(false));});
  server.on("/online", HTTP_GET, [](AsyncWebServerRequest *request){request->send(200, "text/plain", mysensors->onLine());});
  
//  server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request) {request->redirect("/");ESP.reset();});
  server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request) {request->send(200,"text/html","<script>setTimeout(\"top.location='/'\",7000);</script>");delay(1000);tasks.push_back("reset");});
//  server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request) {request->send(200,"text/html","<META http-equiv=\"refresh\" content=\"5;URL=/">");delay(1000);tasks.push_back("reset");});
  server.on("/time", HTTP_GET, [](AsyncWebServerRequest *request) {request->send(200, "text/plain", String(now()));});
  server.on("/_data", HTTP_GET, [](AsyncWebServerRequest *request){

        if(request->method() == HTTP_GET && request->hasParam("setPar")){
          String xxpar = request->getParam("setPar")->value();
          doshell("lb3"+xxpar);
         request->send(304);
         return;

       
        } else if(request->method() == HTTP_GET && request->hasParam("list")) {
          String path = request->getParam("list")->value();
          FSInfo fs_info;
          SPIFFS.info(fs_info);
          Dir dir = SPIFFS.openDir(path);
          path = String();
          if (SPIFFS.exists("/admin/dirs.ini") && !request->hasParam("ref")) {
            request->send(SPIFFS, "/admin/dirs.ini", "text/json");
          } else {
          String output = "[";
          while(dir.next()){
            fs::File entry = dir.openFile("r");
            if (output != "[") output += ',';
            bool isDir = false;
            output += "{\"type\":\"";
            output += (isDir)?"dir":"file";
            output += "\",\"name\":\"";
            output += String(entry.name()).substring(1);
            output += "\",\"size\":\"";
            output += String(entry.size());
            output += "\"}";
            entry.close();
          }
       
            output += ",{\"type\":\"info\",\"name\":\"";
            output += fs_info.totalBytes;
            output += "|";
            output += fs_info.usedBytes;
            output += "\"}]";


            File f = SPIFFS.open("/admin/dirs.ini", "w");
                 f.print(output);
                 f.close();
   //request->send(SPIFFS, "/admin/dirs.ini", "text/json");
 
          request->send(200, "text/json", output);
          output = String();
          }
        
        } else {
      
        Dir dir = SPIFFS.openDir("/data");
          String output = "[";
          while(dir.next()){
            fs::File entry = dir.openFile("r");
            if (output != "[") output += ',';
            bool isDir = false;
            output += "{\"type\":\""; output += (isDir)?"dir":"file"; output += "\",\"name\":\""; output += String(entry.name()).substring(1); output += "\",\"size\":\""; output += String(entry.size()); output += "\"}";
            entry.close();
          } output += "]";
            request->send(200, "text/plain", output);
            output = String();
        }
  });
          
  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");

  server.onNotFound([](AsyncWebServerRequest *request){

    if(request->url().equalsIgnoreCase("/help")){
    charEvent("NOT FOUND");
    
    }   
   
    request->send(404);
  });

  
  server.onFileUpload([](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
    
    
    if(!index) {
      xlock=true;
    xpix(0,conf.clr[3]);
      Serial.printf("UploadStart: %s\n", filename.c_str());
      Serial.printf("%s", (const char*)data);
    sendEvent(String(printf("UploadStart: %s\n", filename.c_str())),"sms");
    sendEvent(String(printf("%s", (const char*)data)),"sms");
    }
    if(final) {
      Serial.printf("UploadEnd: %s (%u)\n", filename.c_str(), index+len);
      sendEvent(String(printf("UploadEnd: %s (%u)\n", filename.c_str(), index+len)),"sms");
 
      xlock=false;
      xpix(0,conf.clr[0]);
    }

  });
  
  server.onRequestBody([](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
    xlock=true;
    if(!index)
      Serial.printf("BodyStart: %u\n", total);
      Serial.printf("%s", (const char*)data);

    sendEvent(String(printf("BodyStart: %u\n", total)),"sms");
    sendEvent(String(printf("%s", (const char*)data)),"sms");

    if(index + len == total)
      Serial.printf("BodyEnd: %u\n", total);
    xlock=false;
  });



  server.begin();
  Serial.println("HTTP Start");
  SSDP.setDeviceType("upnp:rootdevice"); 
  SSDP.setName("Ponika");
  SSDP.setSchemaURL("description.xml");
  SSDP.setHTTPPort(80);
  SSDP.begin();
  Serial.println("SSDP Start");

//  setSyncProvider(getTm);
//  setSyncInterval(600);


configTime(conf.timeZone * 3600, 2*3600, "pool.ntp.org", "time.nist.gov");
delay(1000);
    Serial.println("\nWaiting for time");
    unsigned timeout = 5000;
    unsigned start = millis();
    while (millis() - start < timeout) {
        time_t now = time(nullptr);
        if (now > (2018 - 1970) * 365 * 24 * 3600) {
            
           setTime(time(nullptr));
           Serial.println(now);
           if (!conf.startTime) conf.startTime = now;        
            break;//return;
            
        }
        delay(100);
    }


  loadSensors();
  savedTime = now();
  statex = cron.xloadState();
 
/*
    adc.setGain(ADS1115_REG_CONFIG_PGA_0_256V);
    adc.setSampleRate(ADS1115_REG_CONFIG_DR_8SPS); //Set the slowest and most accurate sample rate

    if (!adc.testConnection()) {
      Serial.println("ADS1115 Connection failed"); //oh man...something is wrong
     } else Serial.println("ADS1115 Connected"); //oh man...something is wrong

*/


 


  if (conf.pcfr_active) {Serial.println("PCF1 Start...");if (statex) pcf1.begin(conf.pcfr_addr,&testWire1,false,statex);else pcf1.begin(conf.pcfr_addr,&testWire1);}
  if (conf.pcfs_active) pcf2.begin(conf.pcfs_addr,&testWire2);
 
//  if (statex && conf.pcfr_active) pcf1.write8(statex); 


  delay(10);
  cron.loadRel();
  cron.startIFTT();
//  pinMode(2,OUTPUT);

for (int i=0;i<16;i++) {if (i>0) pixels.setPixelColor(i-1, 0);pixels.setPixelColor(i, conf.pcfr_color[i]);pixels.show();delay(75);}
for (int i=15;i>-1;i--) {if (i>-1) pixels.setPixelColor(i+1,0);pixels.setPixelColor(i, conf.pcfr_color[i]);pixels.show();delay(75);}
   
for (uint8_t i=1;i<16;i++) {if (!pcf1.read(conf.pcfr_swap[i])) pix(i,0);}
xpix(0,conf.clr[0]);

//digitalWrite(1,LOW);
//delay(500);
//digitalWrite(1,HIGH);

//  getTm();

}


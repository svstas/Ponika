void getTm() {
SyncClient client; 
  if(!client.connect("ponika.org", 80)){
//    sendEvent("Connect Failed","sms");
}
client.setTimeout(2);
client.print("HEAD / HTTP/1.1\r\n\r\n");
  while(!!!client.available()) {yield();}
  while(client.available()){
    if (client.read() == '\n' && client.read() == 'D' && client.read() == 'a' && client.read() == 't' && client.read() == 'e' && client.read() == ':') {client.read();
     String theDate = client.readStringUntil('\r');
     client.stop();
     if (theDate.length()<10) return;
     sendEvent(theDate,"sms");
     theDate = String();

//     delay(1000);
     tcpCleanup();

     return;
    }
  }
}

void SyncData() {
flag_sync = true;


SyncClient client; 


 Serial.println("Start sync");
//sendEvent("Start sync","sms");
  if(!client.connect("ponika.org", 80)){
//    sendEvent("Connect Failed","sms");
Serial.println("Failed");
return;
}
client.setTimeout(2);



  Serial.println("Query");

//  client.print("HEAD / HTTP/1.1\r\n\r\n");
  client.print("GET /esp/set.php?mac=");
  client.print(WiFi.macAddress());
  client.print("&data=");
  client.print(datas.c_str());
  client.println(" HTTP/1.1\r\nHost: ponika.org\r\n\r\n");
//  time_t xxtm = now();
   uint8_t loops = 0;
  while(client.connected() && client.available() == 0){ delay(1);}
//  while(!!!client.available()) {yield();;loops++;if (loops>1000) return;}

  Serial.println("Query1");

  
  while(client.available()){
    if (client.read() == '\n' && client.read() == 'D' && client.read() == 'a' && client.read() == 't' && client.read() == 'e' && client.read() == ':') {client.read();

//     String theDate = client.readStringUntil('\r');

     client.stop();
//     if (theDate.length()<10) return;

//      sendEvent("OK","sms");
     tcpCleanup();
     flag_sync = false;
     return;
    }
  }
  
  
   
}


void doHelp() {
  File f = SPIFFS.open("/admin/help.txt","r");
  if (!f) {sendEvent("Help file is not found ;(","sms"); return;}
 String out = "";
   while (f.available()) {
    out= f.readStringUntil('\n')+"\n";
  sendEvent(out,"sms");

  }
  f.close();
//sendEvent(out,"sms");
out = String();
}



void dotask() {
String xt = tasks.back();tasks.pop_back();

if (xt=="help") {doHelp(); return; }
if (xt=="sync") {SyncData(); return; }
if (xt=="reset") {ESP.reset();}

if (xt=="test") {getTm(); return;}
if (xt=="getList") {mysensors->getList(); return;}

if (xt=="update_spiffs") {
sendEvent("Formatting...","sms");
t_httpUpdate_return ret = ESPhttpUpdate.updateSpiffs("http://ponika.org/Ponika.spiffs.bin");
         switch(ret) {
                case HTTP_UPDATE_FAILED:
                    sendEvent("HTTP_UPDATE_FAILD Error"+ESPhttpUpdate.getLastErrorString(),"sms");
                    break;

                case HTTP_UPDATE_NO_UPDATES:
                    sendEvent("HTTP_UPDATE_FAILD BI","sms");                

                break;
                case HTTP_UPDATE_OK:
                  sendEvent("HTTP_UPDATE_OK","sms");                
                break;
            }
  }

if (xt=="update_bin") {
sendEvent("Formatting...","sms");
t_httpUpdate_return ret = ESPhttpUpdate.update("http://ponika.org/Ponika.bin");
         switch(ret) {
                case HTTP_UPDATE_FAILED:
                    sendEvent("HTTP_UPDATE_FAILD Error"+ESPhttpUpdate.getLastErrorString(),"sms");
                    break;

                case HTTP_UPDATE_NO_UPDATES:
                    sendEvent("HTTP_UPDATE_FAILD BI","sms");                

                break;
                case HTTP_UPDATE_OK:
                  sendEvent("HTTP_UPDATE_OK","sms");                
                break;
            }
  }
}

void tcpCleanup() {
  
  while(tcp_tw_pcbs!=NULL){tcp_abort(tcp_tw_pcbs);}
  }



bool i2ctouch = false;

String listDir(fs::FS &fs, String dirname, uint8_t levels){
    String out = "";
    String xdir = "";

    if (dirname.substring(0,1)!="/") xdir="/"+dirname; else xdir=dirname;
    sendEvent("Listing directory: "+xdir);

    File root = fs.open(&xdir[0]);
    if(!root){
        out+="- failed to open directory";
        return out;
    }
    if(!root.isDirectory()){
        out+=" - not a directory";
        return out;
    }

    File file = root.openNextFile();
    while(file){
      out = "";
        if(file.isDirectory()){
            out+="  DIR : ";
            out+=String(file.name())+"\n";
            if(levels){
                listDir(fs, String(file.name()), levels -1);
            }
        } else {
            out+="  FILE: ";
            out+=String(file.name());//+"\n";
            out+="\tSIZE: ";
            out+=String(file.size())+"\n";
        }
        sendEvent(out);
        file = root.openNextFile();
    }
    return "";
}


String doshell(String msg) {
  msg.toLowerCase();
  if (msg.substring(0,1)=="!") {
    tasks.push_back(msg.substring(1));
    return "";
  }

   if (msg.substring(0,3)=="udp") {
        udpAddress = msg.substring(4); udpAddress.trim();
        return "SET UDP address: "+udpAddress;
   }

   if (msg == "reset") {
         trace("Restarting...");
         sendEvent("Restarting...");
         delay(1000);
         ESP.restart();
    }

    if (msg == "i2c") {tasks.push_back("i2c"); return "";}
        
     if (msg.substring(0,2) == "ls") {
            String path = msg.substring(3); path.trim();          
            if (!path.length()) path="/";
            return listDir(LITTLEFS,path,0);
    }

    if (msg == "free") return "Free heap memory : "+String(ESP.getFreeHeap())+" of "+String(ESP.getHeapSize());

    if (msg == "psram") return "Free PSRAM memory : "+String(ESP.getFreePsram());//+" of "+String(ESP.getPsramSize());

    if (msg == "bat")  {
          return String((analogRead(batPin)) * 3600 / 4095 * 2 + 100);
     }
    if (msg.substring(0,4) == "gpio")  {
     String param = msg.substring(5); param.trim();          
     std::vector<String>args = ssplit(&param[0]," ");
        uint8_t pin = args[1].toInt();
        if (args[0]=="mode") {
            if (args[2] == "input") pinMode(pin,INPUT); else if (args[2] == "output") pinMode(pin,OUTPUT);
            else if (args[2] == "analog") ledcAttachPin(pin, 0);
            args[2].toUpperCase();
            return "GPIO "+String(pin)+" MODE : "+args[2];
        } 
        if (args[0]=="read") {
            return "GPIO "+String(pin)+": "+String(analogRead(pin));
        } 
        
        if (args[0]=="write") {
            digitalWrite(pin,args[2].toInt());
            args[2].toUpperCase();
            return "GPIO "+String(pin)+": "+args[2];
        }

        if (args[0]=="analog") {
            ledcWrite(0, args[2].toInt());
            args[2].toUpperCase();
            return "GPIO "+String(pin)+": "+args[2];
        }

        return "";
//          return String((analogRead(batPin)) * 3600 / 4095 * 2 + 100);
     }

         if (msg == "time") {
//            if (xdisplay) disp(String(printDateTime()));
          return String(printDateTime()).c_str();}
          
         if (msg == "uptime") {return "System running: "+secTime(runtime);}
         if (msg.substring(0,3)=="dls") {
            sdelay = msg.substring(4).toInt();
            return "delay set: "+String(sdelay); 
          }
      
         if (msg == "now") { return String(now());}

if (msg.substring(0,5) == "sleep")  {
     timesleep = msg.substring(6).toInt();// param.trim();          
    sendEvent("Sleeping for every "+String(timesleep)+" seconds");
    vTaskDelay(500);
  esp_sleep_enable_timer_wakeup(timesleep*1000000);
  esp_deep_sleep_start();
}


    if (msg.substring(0,3) == "cat")  {
     String path = msg.substring(4); path.trim();          
     if (path.substring(0,1)!="/") path="/"+path; 
    if (!LITTLEFS.exists(path)) return "File not found :"+path;
      File f = LITTLEFS.open(path, "r");
       while(f.available()) sendEvent(f.readStringUntil('\n'));
      f.close();
     return "";

    }

if (msg=="pnp") {
    mysensors->pnp();
    return "";
}
if (msg=="dev") {
    mysensors->getList();
    return "";
}

if (msg=="bme") {
  BMESensor.refresh(); 
 if (i2ctouch) {i2ctouch=false; BMESensor.refresh();}     
 if (BMESensor.temperature>90 || BMESensor.temperature<-30 || BMESensor.humidity>110 || BMESensor.humidity<5 || BMESensor.pressure>1100 || BMESensor.pressure<900) BMESensor.refresh(); 

  sendEvent("Temperature: "+String(BMESensor.temperature)+"C");                                  // display temperature in Celsius
  sendEvent("Humidity:    "+String(BMESensor.humidity)+"%");
  sendEvent("Pressure:    "+String(BMESensor.pressure  / 100.0F)+"hPa");                           // display pressure in hPa
   return "";
}
         if (msg == "ws") {clid = clidx; return "Starting... ";}
         if (msg == "clid") return String(clid);
//         if (msg == "mic")  micstart = !micstart;

return "command not found.";
}

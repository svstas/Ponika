using namespace std;
using namespace std::placeholders;

void scan_i2c(bool);

bool find_i2c(uint16_t addr) {

for (std::vector<uint16_t>::const_iterator i = conf.i2c.begin(); i != conf.i2c.end(); ++i) if (addr == *i) return true;
return false;
}


void loadConfig(bool sens) {
  
    String par;
    String val;

  scan_i2c(false);
  
  File f = SPIFFS.open("/admin/ini/config.ini", "r");
  if (!f) {Serial.println("Failed to open config.ini");}
  while(f.available()) {
    String xs = f.readStringUntil('\n');
    if (xs.substring(0,1)=="#") continue;
    if (xs.substring(0,1)=="~") {csens.push_back(xs.substring(1)); continue;}
    if (xs.substring(0,1)=="-") {
            uint8_t sep  = xs.indexOf('=');
      String xtp = xs.substring(1,2);
      uint16_t addr = strtol(xs.substring(3,sep).c_str(), NULL, 16);
      xs = xs.substring(sep+1);

     if (xtp=="R") {
        conf.pcfr_addr = addr;
        conf.pcfr_active = find_i2c(addr);
      } else {
        conf.pcfs_addr = addr;
        conf.pcfs_active = find_i2c(addr);
        
      }
     
     char *token = strtok(&xs[0], "|");

        while(token != NULL) {
          
          String tmp = String(token);
          uint8_t sep1  = tmp.indexOf(':');uint8_t sep2  = tmp.lastIndexOf(':');          
          
          uint8_t xpin = tmp.substring(0,sep1).toInt();
          uint8_t ypin = tmp.substring(sep1+1,sep2).toInt();
          uint32_t col = strtol(tmp.substring(sep2+1).c_str(), NULL, 32);
          
          if (xtp=="R") {

                conf.pcfr_color[xpin] = col;
                conf.pcfr_swap[xpin] = ypin;

          } else {

                conf.pcfs_color[xpin] = col;
                conf.pcfs_swap[xpin] = ypin;

            
          }

          token = strtok(NULL,"|");

       };



      }
      
    if (sens)  continue;
    int sep = xs.indexOf('=');
    if (!sep) continue;
    String par = xs.substring(0,sep);
    String val = xs.substring(sep+1);
    par.trim();
    val.trim();

    
    if (par=="WiFiAP") strcpy(conf.WiFiAP,val.c_str());
    if (par=="hostname") strcpy(conf.xhostname,val.c_str());        
    if (par=="hidden" && val!="0") conf.xhidden=true;
    if (par=="ssid") strcpy(conf.ssid,val.c_str());
    if (par=="password") strcpy(conf.password,val.c_str());    
    if (par=="http_username") strcpy(conf.http_username,val.c_str());
    if (par=="http_password") strcpy(conf.http_password,val.c_str());
//  if (par=="sensors" && val=="0") xsensors=false;
//    if (par=="autoreboot") autoreboot=val.toInt();
    if (par=="timeZone") conf.timeZone=val.toInt();
    if (par=="mainLoop") conf.mainLoop=val.toInt();
    if (par=="colors") {

      std::vector<String>cols = ssplit(&val[0],"|");
      int xi=0;
      for (std::vector<String>::const_iterator i = cols.begin(); i != cols.end(); ++i) {
        conf.clr[xi] = strtol((*i).c_str(), NULL, 32); xi++;
        }
      
      std::vector<String>(cols).swap(cols);
    }


// Sensors
  }
   f.close();
    par = String();
    val = String();

return;
}



void savesens() {
cron.xsaveState();
String logname = "/data/"+printDate()+".txt";
          
  File f;
 
         ESP.wdtDisable();

String xrr ="";

    for (uint8_t i=0;i<8;i++) {
      if (cron.irelay[i]!=0 && !cron.rstat[i]) {
/*
    bool st;
      
            switch (cron.irelay[i]) {
                  case 1: st = 1; break;
                  case 2: st = 0; break;            
                  case 3: st = 0; break;            
            };
 
*/
        bool st = 1; if (cron.irelay[i]!=1) st=0;
        //{if (cron.rstat[i]==0) st=1; else st=0;} //else if (cron.irelay[i]==3) {st=!(cron.rstat[i]);}
        xrr+="|"+String(300+i)+":";
        if (st) xrr+="1"; else xrr+=0;
        
        if (cron.orelay[i]==cron.relay[i]) cron.irelay[i]=0; else {cron.irelay[i]=3;cron.orelay[i]=cron.relay[i];}

      }
    }
    
//if (xrr.length()) xrr="|"+xrr;

while(xlock); xlock=true;
xpix(0,conf.clr[2]);
   if (!SPIFFS.exists(logname)) {
        SPIFFS.remove("/admin/dirs.ini");
        
    f = SPIFFS.open(logname, "w");
  //  f.println("Date/Time,TBMP1,TBMP2,THUM,TRTC,HUM,PBMP1,PBMP2,LUX1,LUX2,ANALOG,MEM");
          for (uint8_t i=0;i<8;i++) if (cron.irelay[i]==1) {xrr+="|"+String(300+i)+":1";}
           
          
  } else f = SPIFFS.open(logname, "a");

    f.print(now());  
    f.print("|100:");
    f.print(ESP.getFreeHeap());
    f.print("|");
      

for (uint8_t i=0;i<mysensors->pt;i++) {
  float vall=0;uint8_t rr=0;for (uint8_t j=0;j<12;j++) if (mysensors->LSensors[i].svalue[j]!=0) {vall+=mysensors->LSensors[i].svalue[j];rr++;} vall=vall/rr;

    String xv = String(vall);
    if (xv!="nan") {
        f.print(mysensors->LSensors[i].id);
        f.print(":");
        f.print(xv);
        if (i<(mysensors->pt-1)) f.print("|");
    }
}
    if (xrr) f.print(xrr);
  
    f.println();  
    f.close();

xlock=false;
xpix(0,conf.clr[0]);
  
          ESP.wdtEnable(30);
          sendEvent("Saved sensors "+String(printDateTime()),"sms"); 
        
}



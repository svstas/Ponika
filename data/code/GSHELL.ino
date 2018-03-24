

/*
void sendHello() {
  events.send("Restart after 1m", "sms");
}
*/

bool xbreak = false;


String doshell(String msg)   {

using namespace std;
using namespace std::placeholders;


  
        if (msg == "reset") {
              Serial.println("Restarting...");
              delay(1000);
//              client.stop_abort();
                ESP.restart();
              //ESP.restart();
          return "Restarting...";
          }


      if (msg.substring(0,4)=="free") {
//        sendEvent(String(freeRam()),"sms");
      }
      if (msg.substring(0,6)=="uptime") {

          char tm[] = "000 00:99:00";
//          strcat_P(tm,txt3);
          int secs = mysensors->uptime();
          char buf[100];
//          strcpy(tm,secTime(sec));
          sprintf_P(buf,txt3,secTime(secs).c_str(),ESP.getFreeHeap());
          charEvent(buf);
          return "";
      }
        if (msg.substring(0,7)=="sensors") {

            String xxsens = msg.substring(8);
              xxsens.trim();
               char res[10];
    
              if (xxsens.length()) {


                int i = xxsens.toInt();
                if (i < mysensors->cnt()) {
                for (int g=11;g>-1;g--) {
                    char buff[200];

                        dtostrf(mysensors->LSensors[i].svalue[g], 4, 2, res);
                        sprintf(buff,">%2u : %s; %d",mysensors->LSensors[i].id,res,mysensors->LSensors[i].isint);
                          charEvent(buff);
                  }
                          
                      strcpy(res,mysensors->cget(i));
                      

                            
                } else {
                
                    if (mysensors->getSensId(&xxsens[0])>-1) {
                            int id = mysensors->getSensId(&xxsens[0]);
                                         strcpy(res,mysensors->cget(id)); 
                                         } else strcpy(res,"-");
                }
                      char buff[100];
                        sprintf(buff,">%2u : %s",mysensors->LSensors[i].id,res);
                      charEvent(buff);



 
          } else {
            mysensors->getList();
            xbreak = true;
//            while(xbreak) {mysensors->getList();delay(2000);}

//            sendEvent(String(mysensors->getList()),"sms");
          }
          
        }


        if (msg.substring(0,3) == "pwm") {
         uint8_t pin = msg.substring(3,4).toInt();
//         uint8_t sep = msg.lastIndexOf(" ");
         int cnt = msg.substring(4).toInt();
         pinMode(pin,OUTPUT);
         analogWrite(pin,cnt);
                sendEvent("PWM: "+String(pin)+" : >> "+String(cnt),"sms");
         }

         
         if (msg == "relay") {
          String out="";
                for(uint8_t i=0;i<8;i++) if (pcf1.read(conf.pcfr_swap[i])) out+="+"; else out+="-"; out+=" ";
                sendEvent(out,"sms");
         }

         if (msg == "cronshow") conf.showRel = true;
         if (msg == "cronhide") conf.showRel = false;


        if (msg == "i2c") {
             scan_i2c();
            return "";
          }


        if (msg == "ntp") {
          getNtpTime();
          return "Getit";
        }


        if (msg == "vdd") {
              ESP.wdtFeed();
//              sendEvent(String(readvdd33(),DEC),"sms");
              sendEvent(String(ESP.getVcc(),DEC),"sms");
          
   //          scan_i2c();
            return "";
          }

        if (msg == "1w") {
            ow();
            return "";
          }

        if (msg.substring(0,3) == "set") {

         uint8_t sep = msg.lastIndexOf(" ");
         int cnt = msg.substring(sep+1).toInt();
         msg = msg.substring(3,sep);
         sep = msg.indexOf("-");
         if (sep>0 && sep<msg.length()) {
                  char *params = &msg[0];//.c_str();
                  char *token = strtok(params,"-");
                 while(token != NULL) {
                    char out[10];
                    strcpy(out,token);

                     if (out[0]=='p') {
                      uint8_t pin = String(out).substring(1).toInt();

                      cron.addCron(pin,now(),cnt,true,1); 
                      if (cnt) sendEvent("Add task for PCF:"+String(pin)+"  FOR "+String(cnt)+" secs","sms");
                      else  sendEvent("Turn off PCF:"+String(pin),"sms");
                                       
                       }
                     token = strtok(NULL, "-");
                 }
          }
        } 

        if (msg == "time") {
          return String(printDateTime());
          }

        if (msg == "mem") return String(ESP.getFreeHeap());
        if (msg == "xtime")   return String(now()); 
        if (msg == "fsreal") return String(ESP.getFlashChipRealSize());
        if (msg == "fssize") {
         // fs_info
      
          FSInfo fs_info;
          SPIFFS.info(fs_info);
 
            String out = String(fs_info.totalBytes-fs_info.usedBytes);
           
          return out;
        }
        
        if (msg == "fsspeed")return String(ESP.getFlashChipSpeed());
        if (msg == "fschipid") return String(ESP.getFlashChipId(),HEX);
//        "Flash real id:   %08X\n", ESP.getFlashChipId()

        

        if (msg.substring(0,3) == "pcf") {
          int pin = msg.substring(3,4).toInt();
          int mod = msg.substring(4).toInt();

            if (pin>-1 && pin<8) {

               cron.addCron(pin,now(),mod,true,1); 
               return "PCF: "+String(pin)+"; REAL PIN:"+conf.pcfr_swap[pin]+"; OUTPUT "+mod;
          
            } else return "PCF: Wrong PIN "+String(pin)+" (valid is 0-7);";
          }


        

        if (msg.substring(0,3) == "pcr") {
          int pin = msg.substring(3,4).toInt();
//          int mod = msg.substring(4).toInt();

           char * mmm = "OFF";
            if (pcf2.read(conf.pcfs_swap[pin])) mmm = "ON";
            if (!conf.pcfr_active) return "PCF OFF";
//          if (mod>0) cron.addCron(conf.pcfs_swap[pin],now(),mod,true); else {pcf1.write(conf.pcfs_swap[pin],HIGH);statex=pcf1.read8();        //ВАЖНО
//          cron.addCron(conf.pcfs_swap[pin],now(),mod,true,2); 
//          }

//          sendEvent("]]]"+String(pin)+":"+String(now())+String(mod),"sms");
//          dig(pin,mod);

          return "PCF2: "+String(pin)+": OUTPUT "+mmm;
          }

        if (msg.substring(0,2) == "dw") {
            int todo = msg.substring(2,3).toInt();
            int pin = msg.substring(3).toInt();
             if (todo==3) pinMode(pin,INPUT); else {
              pinMode(pin,OUTPUT);
              if (todo>0) digitalWrite(pin,HIGH); else digitalWrite(pin,LOW);
             }

            
              sendEvent("PIN:"+String(pin)+": "+String(digitalRead(pin)),"sms");
        }
          
        if (msg.substring(0,3) == "pcx") {
          int pin = msg.substring(3,4).toInt();
          int mod = msg.substring(4).toInt();

          
            if (mod==1) pcf2.write(conf.pcfs_swap[pin],HIGH); else pcf2.write(conf.pcfs_swap[pin],LOW);
//          if (mod>0) cron.addCron(pin,now(),mod,true); else {pcf1.write(pin,HIGH);statex=pcf1.read8();        //ВАЖНО
//          cron.addCron(pin,now(),mod,true,2); 
//          }

//          sendEvent("]]]"+String(pin)+":"+String(now())+String(mod),"sms");
//          dig(pin,mod);

          return "PCF2: "+String(pin)+": OUTPUT "+mod;
          }

         if (msg.substring(0,4) == "conf") { 


            sendEvent("Host  = "+String(conf.xhostname),"sms");
            sendEvent("Auth  = name:"+String(conf.http_username)+"; pass:"+String(conf.http_password),"sms");
            String swaps; for (int i=1;i<8;i++) swaps+=String(i)+":"+String(conf.pcfr_swap[i])+';';
            sendEvent("PCF R = addr:"+String(conf.pcfr_addr,HEX)+"; active:"+String(conf.pcfr_active)+"; SWAP: "+swaps,"sms");
            swaps = String(); for (int i=1;i<8;i++) swaps+=String(i)+":"+String(conf.pcfs_swap[i])+';';
            sendEvent("PCF J = addr:"+String(conf.pcfs_addr,HEX)+"; active:"+String(conf.pcfs_active)+"; SWAP: "+swaps,"sms");
            char out[100];
            for (int i=0;i<countof(conf.clr);i++) {

            sprintf(out,"0x%%.%dX:",conf.clr[i]);
//              out << hex << conf.clr[i] << dec << std::endl;
//                std::cout << std::hex << conf.clr[i] << std::dec << std::endl;
//              out+=String(printf(conf.clr[i],HEX))+";";
            }
//            sendEvent("Colors: ","sms");
//            charEvent(out);

            sendEvent("I2C:","sms");
            for (std::vector<uint16_t>::const_iterator i = conf.i2c.begin(); i != conf.i2c.end(); ++i) 
            {
              sendEvent(">>: "+String(*i,HEX),"sms");
            }
            
            /*
  std::vector<uint16_t>i2c;
  
  uint8_t   pcfr_addr;
  uint8_t   pcfs_addr;
  uint16_t  pcfr_color[8] = {0x336699,0x006600,0x006600,0x006600,0x006600,0x006600,0x006600,0x006600};
  uint8_t   pcfr_swap[8] = {0,1,2,3,4,5,6,7};
  uint16_t  pcfs_color[8] = {0x336699,0x006600,0x006600,0x006600,0x006600,0x006600,0x006600,0x006600};
  uint8_t   pcfs_swap[8] = {0,1,2,3,4,5,6,7};

  bool pcfr_active = false;
  bool pcfs_active = false;

  bool xsync = false;
  char xhostname[10] = "ponika";
  char WiFiAP[3] = "0";
  bool xhidden = false;
  char ssid[15] = "Ponika Labs";
  char password[14] = "JosperAdo1212";
  char http_username[8] = "admin12";
  char http_password[8] = "admin21";
uint8_t timeZone = 3;
  bool bsensors=false;
uint32_t startTime = 0;
 bool showRel = false;
 int  mainLoop = 1000;
*/
          
         }

         if (msg.substring(0,2) == "ps") {
              uint8_t sep = msg.indexOf("-");
              uint8_t xfrom=0;
              uint8_t xto=1000000;
              int8_t xrel=-1;
               if (sep>0 && sep<msg.length()) {
                  msg = msg.substring(sep);
                  char *params = &msg[0];//.c_str();
                  char *token = strtok(params,"-");
                 while(token != NULL) {
                    char out[10];
                    strcpy(out,token);

                    if (out[0]=='f') {
                      xfrom = String(out).substring(1).toInt();                    
                    } else if (out[0]=='t') {
                      xto = String(out).substring(1).toInt();                    
                    } else if (out[0]=='0') {
                      cron.maintance = false;
                      return "crond is OFF";
                    } else if (out[0]=='1') {
                      cron.maintance=true;
                      return "crond is ON";
                    } else if (out[0]=='s') {

                      mysensors->getList();
                      return "";
                    } else if (out[0]=='a') {

                        cron.showActive();
                        return "";                 
                    } else if (out[0]=='r') {
                      xrel = String(out).substring(1).toInt();                    
                    }
                   token = strtok(NULL, "-");
                  };
              }
          cron.show(xfrom,xto,xrel);
    //       std::vector<String>(xx).swap(xx);
           }

         if (msg.substring(0,2) == "pr") {
          int pin = msg.substring(2).toInt();
          sendEvent(String(pcf1.read(pin)),"sms");
         }
         if (msg.substring(0,2) == "pw") {
//         int pin = msg.substring(2).toInt();
sendEvent("VV-> "+String(ina219.getBusVoltage_V()),"sms");
sendEvent("VV-> "+String(ina219.getShuntVoltage_mV()),"sms");

//          sendEvent(String(ina219.getBusVoltage_V()),"sms");
         }
}

/*
       if (msg.substring(0,4) == "info") {
      //  String ff = msg.substring(5);ff.trim();
          fsinfo();
         }


        if (msg.substring(0,3) == "dir") {
          String pp = msg.substring(3);
          client->text(String(pp)+"\n"+Dirs(pp));
          }

       if (msg.substring(0,4) == "sync") {
        String ff = msg.substring(5);ff.trim();
          spiffs2sd(ff);
         }
      if (msg.substring(0,6) == "format") {
     //   String ff = msg.substring(5);ff.trim();
          clearsd();
         }
         if (msg.substring(0,2) == "rm") {
          String fname = msg.substring(3);fname.trim();
          xrmfile(fname);
         }
         if (msg.substring(0,5) == "mkdir") {
          String fname = msg.substring(6);fname.trim();
          xmkdir(fname);
         }

}         
*/




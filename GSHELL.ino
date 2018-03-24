

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
              //  ESP.restart();
              ESP.reset();
          return "Restarting...";
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

        if (msg == "update_spiffs" || msg == "update_bin" || msg == "test" || msg == "sync") {
          tasks.push_back(msg);
          return "";
        }


        if (msg == "free") {
          int xbefore = ESP.getFreeHeap();
          char buff[100];
          tcpCleanup();
          sprintf(buff, "%d afrer - %d", xbefore,ESP.getFreeHeap());
          return buff;
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

        if (msg=="help" || msg=="/?" || msg=="?") {tasks.push_back("help");return "";}


        if (msg == "mem") return String(ESP.getFreeHeap());
        if (msg == "xtime")   return String(now()); 
        if (msg == "fsreal") return String(ESP.getFlashChipRealSize());
        if (msg == "freesize") {
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

            char mmm[] = "OFF";
            pcf2.write(conf.pcfs_swap[pin], HIGH);
            if (pcf2.read(conf.pcfs_swap[pin]) == HIGH) char mmm[] = "ON"; else char mmm[] = "OFF";
            if (!conf.pcfr_active) return "PCF OFF---";
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

        if (msg.substring(0,2) == "dr") {
            int pin = msg.substring(2).toInt();
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

         if (msg == "conf") { 


            sendEvent("Host  = "+String(conf.xhostname),"sms");
            sendEvent("Auth  = name:"+String(conf.http_username)+"; pass:"+String(conf.http_password),"sms");
            String swaps; for (int i=1;i<8;i++) swaps+=String(i)+":"+String(conf.pcfr_swap[i])+';';
            sendEvent("PCF R = addr:"+String(conf.pcfr_addr,HEX)+"; active:"+String(conf.pcfr_active)+"; SWAP: "+swaps,"sms");
            swaps = String(); for (int i=1;i<8;i++) swaps+=String(i)+":"+String(conf.pcfs_swap[i])+';';
            sendEvent("PCF J = addr:"+String(conf.pcfs_addr,HEX)+"; active:"+String(conf.pcfs_active)+"; SWAP: "+swaps,"sms");
            sendEvent("Pixel pin:"+String(conf.pixelPin),"sms");
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
            
          return "";          ;
         }

         if (msg.substring(0,2) == "ps") {
              uint8_t sep = msg.indexOf("-");
              uint8_t xfrom=0;
              uint16_t xto=10000;
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

                    //    tasks.push_back("getList");

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

    return "";
           }

         if (msg.substring(0,2) == "pr") {
          int pin = msg.substring(2).toInt();
          sendEvent(String(pcf1.read(pin)),"sms");
         }

         if (msg.substring(0,2) == "pw") {
//         int pin = msg.substring(2).toInt();
//sendEvent("VV-> "+String(ina219.getBusVoltage_V()),"sms");
//sendEvent("VV-> "+String(ina219.getShuntVoltage_mV()),"sms");

//          sendEvent(String(ina219.getBusVoltage_V()),"sms");
        return "";
         }


  return msg + ": command not found. Type help for details";
         
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




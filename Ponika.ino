#include <lwip/tcp_impl.h>
#include <pgmspace.h>
#include <vector>
#include <algorithm>

#include <ArduinoOTA.h>
#include <ESP8266SSDP.h>
#include <ESP8266WiFi.h>
#include <ESP8266NetBIOS.h>
#include <ESP8266mDNS.h>
#include <ESP8266httpUpdate.h>
#include <DNSServer.h>
#include <ESPAsyncWiFiManager.h>
#include <AsyncWebServer.h>
#include "SyncClient.h"
//#include <ESPAsyncHTTPClient.h> 
#include <Adafruit_NeoPixel.h>
//#include <Kalman.h>
#include <FS.h>
#include <TimeLib.h>
#include <Wire.h>


#include <OneWire.h>
#include <DallasTemperature.h>
#include "SparkFunHTU21D.h"
#include <BME280I2C.h>
#include <RWS_BMP180.h>
#include <pcf8574_esp.h>
#include <BH1750.h>

//#include "TSL2561.h"
//#include <Adafruit_TSL2561_U.h>
//#include <Digital_Light_TSL2561.h>
#include "TSL2561.h"

//#include <Adafruit_INA219.h>
#include <SDL_Arduino_INA3221.h>
#include <MCP3421.h> 
#include <MS5611.h>


String datas;
DNSServer dns;
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
AsyncEventSource events("/events");

TwoWire testWire1;
TwoWire testWire2;
PCF857x pcf1;
PCF857x pcf2;


OneWire ourWire(0); /* Ini oneWire instance */
DallasTemperature dsensors(&ourWire);/* Dallas Temperature Library für Nutzung der oneWire Library vorbereiten */
 
const char txt3[] PROGMEM = "Ponika uptime: %12s; freemem: %d";
const char sensHelp[]  PROGMEM = "Connected : %2d sensors\n";
const char sensHelp2[] PROGMEM = "Sensor ID | Dev ID |                      NAME|TXT_NAME | UPT |         VALUE  |\n";
const char sensHelp1[] PROGMEM = "       %2u |     %2u |          %25s | %3u |    %10s  |\n";

const char txt4[] PROGMEM = "|  Task ID  |      PIN  |                 TIME TO DO  |                  REST  |\n";
const char txt5[] PROGMEM = "|       %2u  |       %2u  |        %s  |          %s  |\n";

const char      line[] PROGMEM = "--------------------------------------------------------------------------------\n";

const char txt2[] PROGMEM = "%2u : %12s; %d\n";

const char sens0[]  PROGMEM = "mem|";
const char sens1[]  PROGMEM = "htut";
const char sens2[]  PROGMEM = "htuh";
const char sens3[]  PROGMEM = "bmp1t";
const char sens4[]  PROGMEM = "bmp1p";
const char sens5[]  PROGMEM = "bme2t";
const char sens6[]  PROGMEM = "bme2p";
const char sens7[]  PROGMEM = "bme2h";
const char sens8[] PROGMEM = "inabv";
const char sens9[] PROGMEM = "inama";
const char sens10[] PROGMEM = "analog";
const char sens11[] PROGMEM = "shtt";
const char sens12[] PROGMEM = "shth";
const char sens13[] PROGMEM = "tsl2561";
const char sens14[] PROGMEM = "mcp3421";
const char sens15[] PROGMEM = "xms5611";
const char sens16[] PROGMEM = "xts5611";

const char * const  psensors[] PROGMEM = {sens0, sens1, sens2, sens3, sens4, sens5, sens6, sens7, sens8, sens9, sens10, sens11, sens12, sens13, sens14, sens15, sens16};


//#include <ADS1115_lite.h>
//ADS1115_lite adc(ADS1115_DEFAULT_ADDRESS); //Initializes wire library, sets private configuration variables to ADS1115 default(2.048V, 128SPS, Differential mode between  AIN0 and AIN1.  The Address parameter is not required if you want default

unsigned long endtime ;
int16_t Raw;


bool xlock = false;

#define countof(a) (sizeof(a) / sizeof(a[0]))
String secTime(int stime);
void sendEvent(String zxmsg, char* tp);
void charEvent(const char *zxmsg);
//void loadConfig(bool sens);
void evtSync(bool bstart);
bool flag_sync = false;

std::vector<String> csens;
std::vector<String> tasks;
//String tasks;


using namespace std;
using namespace std::placeholders;

time_t savedTime;



//           Kalman myFilter(0.125,32,1024,0);



/*
  bool PCFInterruptFlag = false;

  void ICACHE_RAM_ATTR PCFInterrupt() {
  PCFInterruptFlag = true;
  for (uint8_t pin=0;pin<15;pin++) {
    int pp;
    if (pcf2.read(pin)==HIGH) pp = 1; else pp = 0;
    sendEvent(">>"+String(pp),"sms");
  }
  }

*/


uint8_t dallasCount = 0;
uint8_t statex = 0;
uint8_t ostate = 0;


struct conf {

  std::vector<uint16_t>i2c;

  uint16_t   pcfr_addr;
  uint16_t   pcfs_addr;
  uint32_t  pcfr_color[8] = {0x336699, 0x006600, 0x006600, 0x006600, 0x006600, 0x006600, 0x006600, 0x006600};
  uint8_t   pcfr_swap[8] = {0, 1, 2, 3, 4, 5, 6, 7};
  uint32_t  pcfs_color[8] = {0x336699, 0x006600, 0x006600, 0x006600, 0x006600, 0x006600, 0x006600, 0x006600};
  uint8_t   pcfs_swap[8] = {0, 1, 2, 3, 4, 5, 6, 7};
  bool pcfr_active = false;
  bool pcfs_active = false;

  // Состояние      Нормальное, запись, еще что-то, и еще  
  uint32_t clr[4] = {0xFFFF00, 0x00FF00, 0xFF0000, 0xFF0000};
  uint8_t pixelPin = 15;
  bool xsync = false;
  char xhostname[10] = "ponika";
  char WiFiAP[3] = "0";
  bool xhidden = false;
  char ssid[15] = "";
  char password[14] = "";
  char http_username[8];
  char http_password[8];
  uint8_t timeZone = 0;
  bool bsensors = false;
  uint32_t startTime = 0;
  bool showRel = false;
  int  mainLoop = 1000;

} conf;

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(16, 15, NEO_GRB + NEO_KHZ800);

void pix(uint8_t pin, bool xon) {

  if (!xon) pixels.setPixelColor(pin, conf.pcfr_color[pin]); else pixels.setPixelColor(pin, 0);
  pixels.show();
}

void xpix(uint8_t pin, uint32_t color) {
  pixels.setPixelColor(pin, color);
  pixels.show();
}




std::vector<String> ssplit(char * str, const char *sep) {
  std::vector<String>vc;
  char *token = strtok(str, sep);
  while (token != NULL) {
    vc.push_back(token);
    token = strtok(NULL, sep);
  };
  return vc;
  std::vector<String>(vc).swap(vc);
}


struct DS {
  uint8_t id;
  String sname;
  DeviceAddress addr;
};


    std::vector<DS> dlslist;


//#####################################################SENSORS CLASS
class Sensors {

    typedef std::function<float()> result;

  private:

    void xpush(uint8_t sid, float sres) {
      LSensors[sid].xsum = LSensors[sid].xsum + LSensors[sid].value;
      if (LSensors[sid].xshift > 4) {
        LSensors[sid].xshift = 0;
        for (uint8_t i = 11; i > 0; i--) LSensors[sid].svalue[i] = LSensors[sid].svalue[i - 1];
        LSensors[sid].svalue[0] = (float)(LSensors[sid].xsum / 5);
        LSensors[sid].xsum = 0;
      } else LSensors[sid].xshift = LSensors[sid].xshift + 1;
      LSensors[sid].svalue[0] = sres;

      return;
    }

    bool canUpdate() {
      //        if (millis()-mupdate<900) return true; else return false;
      return true;
    }

    // long mupdate = millis();

  public:


    int uptime() {
      return (now() - conf.startTime);
    }

    struct Sensor {

      uint8_t id;
      float value = 0;
      float oldvalue = 0;
      bool isint = false;
      float xsum = 0;
      long last_update;
      uint8_t spointer = 0;
      uint8_t xshift = 0;
      uint8_t uptime;
      uint8_t sdelay = 1;
      bool  kalman = false;
      std::vector<uint8_t> relays;
      bool  changed = false;
      result fres;
      float svalue[12] = {};
      //    char xname[15] = "";
      //    char Sname[1] = "";
      char formula[10] = "X";

    };

    std::vector<Sensor> LSensors;
    //  Sensor LSensors[12] = {};


    // char names[20][17];
    uint8_t pt = 0;
    uint8_t ptr = 0;
    uint8_t snow = 0;


    int checkid(uint8_t id) {
      for (uint8_t i = 0; i < pt; i++) {
        if (LSensors[i].id == id) return i;
      }
      return -1;
    }


    int8_t getSensId(char * sname) {

      if (strncmp(sname, "time", 4) == 0) return 100;
      if (strncmp(sname, "ds", 2) == 0) {
        for (uint8_t i = 0; i < dlslist.size(); i++) {
          if (dlslist[i].sname == String(sname)) return i;
        }
        return 231;
      }

      for (uint8_t i = 0; i < countof(psensors); i++) {
        int id = checkid(i);
        if (id < 0) continue;
        if (strcmp_P(sname, psensors[i]) == 0) return id;
      }
      return 231;
    }


    void xinit() {

      for (uint8_t i = 0; i < csens.size(); i++) {
        uint8_t sep = csens[i].indexOf("=");
        if (!sep) continue;
        String par = csens[i].substring(0, sep);
        String val = csens[i].substring(sep + 1);
        par.trim();
        val.trim();
        uint8_t id = getSensId(&par[0]);
        if (id > -1 && id < 100) {
          std::vector<String>vc = ssplit(&val[0], "|");
          //            par.toCharArray(LSensors[id].xname,10);

          //            LSensors[id].Sname = vc[0];
          //              sprintf(LSensors[id].Sname,"%s",vc[0].c_str());
          //            vc[0].toCharArray(LSensors[id].Sname,18);
          LSensors[id].id = vc[0].toInt();
          vc[2].toCharArray(LSensors[id].formula, 10);

          //"X+1";
          if (vc[3].toInt() > 0) LSensors[id].sdelay = vc[3].toInt();
          if (vc[4] == "int") LSensors[id].isint = true;
          std::vector<String>(vc).swap(vc);

        }
      }

    }

    bool block = false;

    uint8_t cnt() {
      return pt;
    }
    void xclear() {
      pt = 0;
    }



    String loadNames(bool toscreen) {
      //  loadConfig(true);
      String out;//= "mem@100||100|Память|X|1|int|10000|32000|||||";


      for (uint8_t i = 0; i < csens.size(); i++) {
        String sss = csens[i];
        int sep = sss.indexOf("=");
        String ssname = sss.substring(0, sep); ssname.trim();
        sss = sss.substring(sep + 1); sss.trim();
        out += ssname + "@";
        uint8_t sid = getSensId(&ssname[0]);

        String rels = "";

        if (sid > -1 && sid < 200) {

          out += String(LSensors[sid].id);

          uint8_t rsize = LSensors[sid].relays.size();
          for (uint8_t i = 0; i < rsize; i++) {
            rels += LSensors[sid].relays[i] + 1;
            if (i < rsize - 1) rels += ",";
          };
        } else out += "-1";



        out += "|" + rels + "|" + sss;
        out += "|";
        if (i < csens.size() - 1) out += "#";

      }

      if (toscreen) {
        sendEvent(out, "x");
        out = String();
        evtSync(true);
      } else return out;
      out = String();
    }

    void getList() {
      char buff[300];
      sprintf_P(buff, sensHelp, pt);
      charEvent(buff);
      snprintf_P(buff, countof(buff), line); charEvent(buff);
      snprintf_P(buff, countof(buff), sensHelp2); charEvent(buff);
      snprintf_P(buff, countof(buff), line); charEvent(buff);

      for (uint8_t i = 0; i < pt; i++) {

//        if (LSensors[i].id==13) continue;
        //       buff[0]=0;
        char se[30];
        char res[10]; strcpy(res, cget(i));
        //      charEvent(res);
        if (LSensors[i].id < 50) strcpy_P(se, psensors[LSensors[i].id]); else strcpy(se, dlslist[LSensors[i].id - 50].sname.c_str());
        //  const char sensHelp1[] PROGMEM = "Sensor ID: %02u; %20s; %3u; %2u; %10u; %6s; %20s; %20s;\n";
        //  const char sensHelp[] PROGMEM  = "Connected : %02d sensors\nSensor ID: id;nick;uptime;refresh;last;formula;value;Name\n

        snprintf_P(buff, countof(buff), sensHelp1, i, LSensors[i].id, se, LSensors[i].uptime, res);
        charEvent(buff);
      }
      snprintf_P(buff, countof(buff), line); charEvent(buff);

    };



    int16_t getVal(char* sname) {
      return LSensors[getSensId(sname)].value;// else return "0";

    }

    bool setVal(char * sname,long value) {
      uint8_t id = getSensId(sname);
       if (id) LSensors[getSensId(sname)].value = value; else return false;
     return true;
    }

    float getValId(uint8_t id) {
      if (id > -1 && id < pt) {
        return LSensors[id].value;
      } else return -111;
    }


    char * cget(uint8_t id) {
      char buf[10] = "-";
      char * rbuf = buf;
      if (id > -1 && id < 200) {
        //      sendEvent(">>"+String(id)+":"+String(LSensors[id].value),"sms");

        if (LSensors[id].isint) itoa(LSensors[id].value, buf, 10); else dtostrf(LSensors[id].value, 8, 2, buf);

        //        charEvent(buf);
        return rbuf;
      } else return rbuf;
    }

    /*
      bool updateSensor(char* sname) {
            updateSensorId(getSensId(sname));
          return true;
      }
    */

    int p = 0;
    

    bool updateSensorId(uint8_t id) {

      //        if ((now()-LSensors[id].last_update)<LSensors[id].sdelay) return true;

      //         LSensors[id].value = LSensors[id].fres();

      LSensors[id].last_update = now();
      //         xpix(0,conf.clr[1]);
      float res = (float)LSensors[id].fres();//+p; p++;
      if (id>49 && id<100 && (res==-127 || res==85)) res = LSensors[id].value;          //TODO Err handling
   //   if (LSensors[id].kalman) res = myFilter.getFilteredValue(res);
      LSensors[id].value = res;
      if (LSensors[id].value == LSensors[id].svalue[0]) LSensors[id].changed = false; else LSensors[id].changed = true;
      xpush(id, res);
      //         xpix(0,conf.clr[0]);
      return true;
    }

    char* getSensP (char* sname) {
      for (auto psens : psensors) {

        charEvent(psens);
      }


    }

    

    void addSensor(const char * sname, uint8_t xdelay, std::function<float()> callback, bool isint, uint8_t id) {
      Sensor one;
      one.id = id;
      one.isint = false;
//      if (id==10) one.kalman=true;
      one.sdelay = xdelay;
      one.fres = callback;
      time_t time;
      time = millis();
      one.value = callback();
      time = millis() - time;
      one.uptime = time;
      LSensors.push_back(one);
      if (id > 49 && id<100) {
        DS n;
        n.sname = String(sname);
       dlslist.push_back(n);
       one.id=50+dlslist.size();
      }
      pt++;
    }

    //  uint8_t count=0;

    bool lamp = LOW;
    void maintance() {

      uint32_t xmillis;
      xmillis = millis();

      if (!((now() - LSensors[snow].last_update) < LSensors[snow].sdelay)) {

        if (LSensors[snow].last_update - xmillis < LSensors[snow].sdelay * 1000) {
          snow++;
          if (snow > pt - 1) snow = 0;
          return;
        }

        //if (!(LSensors[snow].id > 49 && LSensors[snow].id < 100))
        updateSensorId(snow);
        
        LSensors[snow].uptime = millis() - xmillis;
      }

      snow++; if (snow > pt - 1) snow = 0;

    }

    String onLine() {
      unsigned long sstime = now() - 65;
      String mem = String(ESP.getFreeHeap());
      String out;
      for (uint8_t j = 12; j > 0; j--) {
        sstime = sstime + 5;
        out += String(sstime) + "|" + mem;

        for (uint8_t i = 0; i < pt; i++) {
          if (LSensors[i].isint)  out += "|" + String(LSensors[i].id) + ":" + String((int)LSensors[i].svalue[j - 1]);
          else  out += "|" + String(LSensors[i].id) + ":" + String(LSensors[i].svalue[j - 1]);
        }
        out += "\n";
      }
      return out;
      out = String();
    }

};



Sensors *mysensors = new Sensors();

void evtSync(bool bstart) {

  String out;
  time_t stm = now();
  if (stm-savedTime>1 && flag_sync) {
//    sendEvent("SS","sms");
    ESP.reset();
  }
  out = String(stm) + "|" + String(ESP.getFreeHeap());
  for (uint8_t i = 0; i < mysensors->pt; i++) {
    if (mysensors->LSensors[i].oldvalue == mysensors->LSensors[i].value && !bstart) continue;
    mysensors->LSensors[i].oldvalue = mysensors->LSensors[i].value;
    if (mysensors->LSensors[i].isint)  out += "|" + String(mysensors->LSensors[i].id) + ":" + String((int)mysensors->LSensors[i].value);
    else  out += "|" + String(mysensors->LSensors[i].id) + ":" + String(mysensors->LSensors[i].value);
  }

  sendEvent(out, "s");
  out = String();
  savedTime = stm;
}
//#######################################################################################

typedef std::function<bool()> Bool;

// CRON!!!
int cronshift(uint8_t sens, float ival, bool more, String stime) {
  // Должна вернуть значение на сколько именно больше или меньше условие

}

bool tcheck(String sival, String par, bool more) {
  bool res = !more;
  for (uint8_t i = 0; i < par.length(); i++) {

    int ival = sival.substring((i + 1) * 2 - 2, (i + 1) * 2).toInt();
    char c = par.charAt(i);
    int rr;
    //setTime();
    
    if (c == 'M')      rr = month(now());
    else if (c == 'D') rr = day(now());
    else if (c == 'h') rr = hour(now()); //+conf.timeZone; if (rr>23) rr=rr-24;}
    else if (c == 'm') rr = minute(now());
    else if (c == 's') rr = second();

 //   if (c == 'M')      rr = month(now() + conf.timeZone * 3600);
 //   else if (c == 'D') rr = day(now() + conf.timeZone * 3600);
 //   else if (c == 'h') rr = hour(now() + conf.timeZone * 3600); //+conf.timeZone; if (rr>23) rr=rr-24;}
 //   else if (c == 'm') rr = minute(now() + conf.timeZone * 3600);
 //   else if (c == 's') rr = second();

    if (ival == rr) continue; else {

      res = ival < rr;

      if (res == more) return true; else return false;
    }
  }
  return (res == more);
}

bool scheck(uint8_t sens, float ival, bool more) {
  int32_t xval = (long)((ival) * 100);
  int32_t yval = (long)((mysensors->getValId(sens)) * 100);
  bool res = xval > yval; if (more) res = !res;
  return res;
}


bool readPcf(uint8_t pin) {
}



class Cron {

  private:

    struct Func {
      uint8_t pin;
      uint8_t xid = 0;
      //  Bool f;
      uint8_t sensor;
      uint16_t offtime;
      //  bool tchk = false;
      bool more = false;
      char hms[5];
      char val[4];

    };

    struct Lcron {
      uint8_t xpcf;
      uint8_t pin;
      uint32_t tstart;
      uint32_t tend;
    };

    std::vector<Lcron> vc;

    //Lcron vc[16] = {};
    int ptr = 0;

    std::vector<Func> Xfunc;

    void setTask(String sx, uint8_t pin, uint8_t rtime, uint8_t xid) {

      //  char task[10];
      //  sx.toCharArray(task,10);
      bool more = true;

      String sens = "";
      String val = "";

      uint8_t sep = sx.indexOf(">");
      if (sep && sep < sx.length()) {
        sens = sx.substring(0, sep);
        val = sx.substring(sep + 1);
      } else {
        sep = sx.indexOf("<");
        if (sep && sep < sx.length()) {
          more = false;
          sens = sx.substring(0, sep);
          val = sx.substring(sep + 1);
        }
      }

      uint8_t sensor = mysensors->getSensId(&sens[0]);
      String par = "";
      if (sensor == 100) {
        sep = sens.indexOf(".");
        if (sep > 0 && sep < sens.length()) par = sens.substring(sep + 1);
      } else mysensors->LSensors[sensor].relays.push_back(pin);

      Func nf;
      nf.pin = pin;
      nf.xid = xid;
      nf.sensor = sensor;
      nf.offtime = rtime;
      strcpy(nf.val, val.c_str());
      nf.more = more;
      strcpy(nf.hms, par.c_str());
      Xfunc.push_back(nf);

      sens = String(); sx = String(); par = String();
    }

    void sort() {
      std::sort(vc.begin(), vc.end(), [] (Lcron const & a, Lcron const & b) {
        return a.tstart < b.tstart;
      });
    }


  public:

    void xsaveState() {
      File f;
      statex = pcf1.read8();
      f = SPIFFS.open("/admin/ini/state.ini", "w");
      if (!f) {
        Serial.println("Failed to open state.ini");
      }
      f.println("#" + String(statex) + ":" + now());
      for (uint8_t i = 0; i < ptr; i++) {
        f.println(String(vc[i].pin) + ":" + String(vc[i].tstart) + ":" + String(vc[i].tend));
      }
      f.close();
    };


    bool relay[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    bool orelay[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    //uint8_t irelay[8] = {1,1,1,1,1,1,1,1};
    //uint8_t irelay[8] = {0,0,0,0,0,0,0,0};
    uint8_t irelay[8] = {1, 1, 1, 1, 1, 1, 1, 1}; //инверсия (наоборот с rstat)
    bool rstat[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    //bool rstat[8] = {1,1,1,1,1,1,1,1};

    bool maintance = true;

    void loadRel() {
      ostate = statex;
      for (uint8_t i = 0; i < 8; i++) {
        relay[i] =  pcf1.read(conf.pcfr_swap[i]);
        orelay[i] = relay[i];
      }
    }

    uint8_t xloadState() {
      File f;
      uint8_t statexx;
      if (SPIFFS.exists("/admin/ini/state.ini")) f = SPIFFS.open("/admin/ini/state.ini", "r"); else return 0;
      if (!f) {
        Serial.println("Failed to open state.ini");
        return 0;
      }
      uint8_t cc = 0;
      while (f.available()) {
        if (cc > 20) continue; cc++;

        String xs = f.readStringUntil('\n');
        uint8_t sep;
        if (xs.substring(0, 1) == "#") {
          //    const char *ss;
          sep = xs.indexOf(":");
          if (!sep) {
            statexx = xs.substring(1).toInt();
          } else {
            statexx = xs.substring(1, sep).toInt();
            savedTime = xs.substring(sep + 1).toInt();
            if (savedTime > 0 && savedTime > (conf.startTime+2)) setTime(savedTime); // conf.startTime=now();

          }
          continue;
        }
        sep = xs.indexOf(":");
        uint8_t pin = xs.substring(0, sep).toInt();
        if (!sep) continue;
        uint8_t    sep1 = xs.indexOf(":", sep + 1);
        uint32_t tstart = xs.substring(sep + 1, sep1).toInt();
        uint16_t    tend = xs.substring(sep1 + 1).toInt();
        addCron(pin, tstart, tend, false, 1);
        xs = String();

      }
      if (ptr > 1)sort();
      f.close();
      return statexx;
    }


    void addCron(uint8_t pin, uint32_t tstart, uint32_t tend, bool save, uint8_t xpcf) {
      //if (xpcf==1) pin = conf.pcfr_swap[pin]; else pin = conf.pcfr_swap[pin];
      if (!tstart) return;
      vc.push_back({xpcf, pin, tstart, tend});
      ptr++;
      if (ptr > 1) sort();
      if (save) xsaveState();
    }

    void delC() {
      ptr--;
      vc.erase(vc.begin());
      vector<Lcron>(vc).swap(vc);
    }


    void sendR(bool xxxstart) {
      char buf[8] = {};

      for (int i = 0; i < 8; i++) {

        if (irelay[i] == 0) {
          if (orelay[i] != relay[i]) {
            if (relay[i] == rstat[i]) irelay[i] = 1;
            else irelay[i] = 2;
            orelay[i] = relay[i];
          }
        }


        const char * ron;
        const char * rof;
         
        if (rstat[i]) {
          
        ron = "-";
        rof = "+";

       
        } else {

        ron = "+";
        rof = "-";
       
        }
        if (relay[i] == 1) strcat(buf, rof); else strcat(buf, ron);
      }

      if (xxxstart) ostate = 0;
      if (statex != ostate) {
        charEventR(buf);
        ostate = statex;
      }
    }


    bool mycheck(uint8_t i) {

      String hms = Xfunc[i].hms;
      String val = Xfunc[i].val;
      uint8_t sensor = Xfunc[i].sensor;

      if (sensor == 100) return tcheck(val, hms, Xfunc[i].more);
      else {
        return scheck(sensor, val.toFloat(), Xfunc[i].more);
      }
      return false;
    }

    // ############################################### Перегруз
    void doCron() {

      bool   tmpr[8] = {0, 0, 0, 0, 0, 0, 0, 0};
      int  startt[8] = {0, 0, 0, 0, 0, 0, 0, 0};
      int    xoff[8] = {0, 0, 0, 0, 0, 0, 0, 0};
      int  xoldid[8] = { -1, -1, -1, -1, -1, -1, -1, -1};
      bool save = false;
      bool rchange = false;
      bool isOn;
      std::vector<uint8_t> rcoff;

      if (ptr > 0) isOn = rstat[vc[0].pin];

      if (ptr > 0 && now() >= vc[0].tstart) {

        if (relay[vc[0].pin] != isOn && (vc[0].tend > 0)) {
          if (vc[0].xpcf == 1) {

            if (conf.pcfr_active) pcf1.write(conf.pcfr_swap[vc[0].pin], isOn);
            pix(vc[0].pin, isOn);
          } else {

            pcf2.write(conf.pcfs_swap[vc[0].pin], isOn);

          }

          relay[vc[0].pin] = isOn;
          rchange = true;
          //  sendEvent("ADD>>"+String(vc[0].tstart)+":"+String(now()+vc[0].tend),"sms");

          uint32_t tgo = vc[0].tend + now();
          uint8_t gpin = vc[0].pin;

          if (ptr > 0) delC();
          addCron(gpin, tgo, 0, false, 1);

        } else {

          while (ptr > 0 && now() >= vc[0].tstart) {

            rcoff.push_back(vc[0].pin);
            relay[vc[0].pin] = (!(rstat[vc[0].pin]));
            if (ptr > 0) delC();
          }

        }
        save = true;
      }


      int xskip = -1;
      for (int i = 0; i < Xfunc.size(); i++) {

        uint8_t pin = Xfunc[i].pin;
        uint8_t  id = Xfunc[i].xid;
        xoff[pin] = Xfunc[i].offtime;

        bool isOn = rstat[pin];

        if (relay[pin] == isOn || xskip == id) continue;             // Если уже стоит ;)

        if (startt[pin] == 0) {

          startt[pin] = 1; tmpr[pin] = 1; xoldid[pin] = id;
          //   sendEvent(">>>"+String(pin,DEC)+":"+String(xoldid[pin],DEC)+":"+String(id,DEC),"sms");

        }

        if (xoldid[pin] != id) {
          xoldid[pin] = id;
          startt[pin]++;
          tmpr[pin] = (tmpr[pin] || mycheck(i));
          if (!tmpr[pin]) {
            xskip = id;
            continue;
          }

        } else {
          xoldid[pin] = id;
          tmpr[pin] = (tmpr[pin] *  mycheck(i));
          if (!tmpr[pin]) {
            xskip = id;
            continue;
          }
        };
      };


      for (int i = 0; i < 8; i++) {
        bool isOn = rstat[i];
        if (startt[i] == 0 ) continue;

        if (tmpr[i] && relay[i] != isOn) {
          //  sendEvent("SHIT!>>"+String(i)+":"+String(Xfunc[i].offtime)+":"+String(tmpr[i])+":"+String(relay[i]),"sms");
          if (conf.pcfr_active) pcf1.write(conf.pcfr_swap[i], isOn);
          pix(i, isOn);

          relay[i] = isOn;
          rchange = true;
          addCron(i, now() + xoff[i] - 1, 0, false, 1);
          save = true;

        }
      }

      while (!rcoff.empty()) {
        uint8_t rec = rcoff.back();
        rcoff.pop_back();
        if (tmpr[rec]) continue;
        relay[rec] = !(rstat[rec]);
        if (conf.pcfr_active) pcf1.write(conf.pcfr_swap[rec], (!(rstat[conf.pcfr_swap[rec]])));
        pix(rec, (!(rstat[rec])));
        rchange = true;
      }


      if (save)  xsaveState();
      if (rchange) {
        sendR(false);
      }

    }



    void startIFTT() {

      File f;

      if (SPIFFS.exists("/admin/ini/iftt.ini")) f = SPIFFS.open("/admin/ini/iftt.ini", "r"); else return;
      if (!f) {
        Serial.println("Failed to open state.ini");
        return;
      }
      Xfunc.clear();
      int id = 0;
      while (f.available()) {
        String xs = f.readStringUntil('\n');
        if (xs.substring(0, 1) == "#" || xs.length() < 5) continue;
        uint8_t sep = xs.indexOf("=");
        if (!sep) continue;
        String tmp = xs.substring(0, sep);
        String val = xs.substring(sep + 1); val.trim();
        sep = tmp.indexOf(":");
        uint8_t pin = xs.substring(0, sep).toInt();
        int rtime = xs.substring(sep + 1).toInt();
        std::vector<String>vc;
        char *token = strtok(&val[0], "|");
        while (token) {
          vc.push_back(token);
          token = strtok(NULL, "|");
        };
        for (uint8_t i = 0; i < vc.size(); i++) {
          setTask(vc[i], pin, rtime, id);
        }
        id++;
        val = String(); vc.clear();
        std::vector<String>(vc).swap(vc);
      }
      f.close();

      doCron();
    }


    void showActive() {
      char buff[200]; snprintf_P(buff, countof(buff), line); charEvent(buff);
      snprintf_P(buff, countof(buff), txt4); charEvent(buff);
      snprintf_P(buff, countof(buff), line); charEvent(buff);
      for (uint8_t i = 0; i < ptr; i++) {
        int xtime = vc[i].tstart - now();
        if (xtime < 0) xtime = xtime * -1;
        snprintf_P(buff, countof(buff), txt5, i, vc[i].pin, printDateTimeT(vc[i].tstart).c_str(), secTime(xtime).c_str()); charEvent(buff);
      }
      snprintf_P(buff, countof(buff), line); charEvent(buff);
    }

    void show(uint8_t from, uint8_t to, int8_t rel) {

      int oldd = 0;
      char buff[100];

      snprintf_P(buff, countof(buff), line); charEvent(buff); //PRINT LINE

      snprintf_P(buff, countof(buff), " >>> %02u ", Xfunc.size()); charEvent(buff);

      if (to > Xfunc.size()) to = Xfunc.size();
      for (uint8_t zi = from; zi < to; zi++) {

        if (rel > -1) if (Xfunc[zi].pin != rel) continue;

        char buff[100];
        const char *xx[2] = {"OFF", " ON"};
        uint8_t rx =  mycheck(zi);
        //  Serial.println(zi);


        if (oldd != Xfunc[zi].xid) {
          //buff[0]=0;
          snprintf_P(buff, countof(buff), line); charEvent(buff);
          //buff[0]=0;
        }

        oldd = Xfunc[zi].xid;
//        time_t t = now() + 3600 * conf.timeZone;
        
        time_t t = now();
        snprintf_P(buff,
                   100,
                   //           PSTR("ID: %3u; TaskID : %02d ;  pin : %02d;   sensor : %02d; STATUS : %s"),
                   "Time: %02u:%02u:%02u  ID : %3d  ; TaskID : %2u;  pin : %2u;  sensor : %3u; STATUS : %s",
                   hour(t),
                   minute(t),
                   second(t),
                   zi,
                   Xfunc[zi].xid,
                   Xfunc[zi].pin,
                   Xfunc[zi].sensor,
                   xx[rx]
                  );

        //    Serial.println(String(buff));
        charEvent(buff);

      }


      return;

    }



} cron;



//#######################################################################################

int sec = 0, osec = 0;


uint8_t badcount = 60;

//time_t xloop = millis();


int xr = 0;
int xg = 128;
int xb = 128;
int sr = 1;
int sg = -1;
int sb = 1;

void ipix() {
  //return;
  if (!conf.startTime) {
    if (sec != osec) {
      pixels.setPixelColor(0, 0);
    } else pixels.setPixelColor(0, 255, 0, 0);
    pixels.show();
  } else {
    pixels.setPixelColor(0, xr, xg, xb);
    pixels.show();
    xr = xr + sr;
    xg = xg + sg;
    xb = xb + sb;
    if (xr > 254 || xr < 1) sr = sr * -1;
    if (xb > 254 || xb < 1) sb = sb * -1;
    if (xg > 254 || xg < 1) sg = sg * -1;

  }

}


void loop() {

//savedTime = now();
 /* 
     for (int i=0;i<dallasCount;i++) {
   
          DeviceAddress dallas;
          if (dsensors.getAddress(dallas, i) && dsensors.isConversionAvailable(dallas)) {
       
                char dname[18] = "ds";
                strcat(dname,printAddress(dallas).c_str()); 

                  if (mysensors->setVal(dname,dsensors.getTempC(dallas)))  {
                  //  sendEvent(dname+":"+dsensors.getTempC(dallas),"sms");
                  
              dsensors.requestTemperatures();
                  
//                    dsensors.requestTemperaturesByAddress(dallas);
                   }
                  
              
              }
     }

*/

  ArduinoOTA.handle();
  sec = second();

//  ipix();
  if (tasks.size()) dotask();

  /*
    if(PCFInterruptFlag){
      sendEvent("INTERRUPT","sms");
      PCFInterruptFlag=false;
    }
  */

  if (conf.startTime) mysensors->maintance();


  if (sec != osec) {

    if (cron.maintance) cron.doCron();

    if ((sec == 1) && cron.maintance) {
      savesens();
    }


    evtSync(false);

    //    if (dallasCount) dsensors.requestTemperatures();

  }

  osec = sec;

}

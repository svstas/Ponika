String doshell(String msg);
void scan_i2c(bool out);

#include <BME280_t.h>                                                   // import BME280 template library
#define ASCII_ESC 27
#define MYALTITUDE  150.50
char bufout[10];
BME280<> BMESensor; 
bool bmeInited = false;
time_t lastcheck = now();

float getBME(uint8_t tp) {
  if (lastcheck!=now()) {
      BMESensor.refresh();
      lastcheck = now();
      if (BMESensor.temperature>90 || BMESensor.temperature<-30 || BMESensor.humidity>110 || BMESensor.humidity<5 || BMESensor.pressure>1100 || BMESensor.pressure<900) BMESensor.refresh(); 
  }
        switch(tp) {
          case 1: return BMESensor.temperature;
          case 2: return BMESensor.humidity;
          case 3: return BMESensor.pressure / 100.0F;
     }
        return 0;
}


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

std::vector<String> csens;

/*
struct DS {
  uint8_t id;
  String sname;
  DeviceAddress addr;
};
std::vector<DS> dlslist;
*/

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
      return runtime;
    }

    struct Sensor {

      uint8_t id;
      char name[20] = "";
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

     if (strncmp(sname, "time", 4) == 0) return 1;

     for (uint8_t i = 0; i < pt; i++) {
        if (strcmp_P(sname, LSensors[i].name) == 0) return i;
      }
      return 0;
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
          LSensors[id].id = vc[0].toInt();
          vc[2].toCharArray(LSensors[id].formula, 10);
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
//        evtSync(true);
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

      for (uint8_t i = 0; i < pt-1; i++) {
//            sendEvent(String(i));
//        if (LSensors[i].id==13) continue;
        //       buff[0]=0;
//        char se[30];
        char res[10]; strcpy(res, cget(i));
        //      charEvent(res);
//        if (LSensors[i].id < 50) strcpy_P(se, psensors[LSensors[i].id]); else strcpy(se, dlslist[LSensors[i].id - 50].sname.c_str());
//        strcpy_P(se, psensors[LSensors[i].id]); 
//        strcpy(se, LSensors[i].name);
        //  const char sensHelp1[] PROGMEM = "Sensor ID: %02u; %20s; %3u; %2u; %10u; %6s; %20s; %20s;\n";
        //  const char sensHelp[] PROGMEM  = "Connected : %02d sensors\nSensor ID: id;nick;uptime;refresh;last;formula;value;Name\n

        snprintf_P(buff, countof(buff), sensHelp1, i, LSensors[i].id, LSensors[i].name, LSensors[i].uptime, res);
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

//    int p = 0;
    

    bool updateSensorId(uint8_t xid) {

      //        if ((now()-LSensors[id].last_update)<LSensors[id].sdelay) return true;

      //         LSensors[id].value = LSensors[id].fres();

      LSensors[xid].last_update = now();
      //         xpix(0,conf.clr[1]);
      float res = (float)LSensors[xid].fres();//+p; p++;
//        sendEvent(String(xid)+LSensors[xid].name);
//        float res = 100;
   //   if (id>49 && id<100 && (res==-127 || res==85)) res = LSensors[id].value;          //TODO Err handling
   //   if (LSensors[id].kalman) res = myFilter.getFilteredValue(res);
      LSensors[xid].value = res;
      if (LSensors[xid].value == LSensors[xid].svalue[0]) LSensors[xid].changed = false; else LSensors[xid].changed = true;
      xpush(xid, res);
      //         xpix(0,conf.clr[0]);
      return true;
    }

    void getSensP (char* sname) {
      for (auto psens : psensors) {

        charEvent(psens);
      }

    }

    
    

    void addSensor(const char * sname, uint8_t xdelay, std::function<float()> callback, bool isint, uint8_t id) {
      Sensor one;
      one.id = id;
      one.isint = false;
//      one.name = sname.c_str();
      strcat(one.name,sname);
//      snprintf(one.name, "%c", sname);
//      one.name = sname;
//      if (id==10) one.kalman=true;
      one.sdelay = xdelay;
      one.fres = callback;
      time_t time;
      time = millis();
      one.value = callback();
      time = millis() - time;
      one.uptime = time;
      LSensors.push_back(one);
/*      
      if (id > 49 && id<100) {
        DS n;
        n.sname = String(sname);
       dlslist.push_back(n);
       one.id=50+dlslist.size();
      }
*/
      pt++;
    }

    //  uint8_t count=0;

    void maintance() {

      if (!sensloaded) return;

      uint32_t xmillis;
      xmillis = millis();

      if (!((now() - LSensors[snow].last_update) < LSensors[snow].sdelay)) {

        if (LSensors[snow].last_update - xmillis < LSensors[snow].sdelay * 1000) {
          snow++;
          if (snow > (pt - 2)) snow = 0;
          return;
        }

//        LSensors[snow].last_update=now();
        //if (!(LSensors[snow].id > 49 && LSensors[snow].id < 100))
//        sendEvent("upd"+String(snow)+" - "+String(pt));

        updateSensorId(snow);
        
        LSensors[snow].uptime = millis() - xmillis;
        vTaskDelay(20);

      }

      snow++; if (snow > (pt - 2)) snow = 0;

    }

    

    void pnp() {
    sensloaded=false;
    vTaskDelay(50);
    std::vector<Sensor>(LSensors).swap(LSensors);
    LSensors.clear();
    pt = 1;

     scan_i2c(false);
     vTaskDelay(100);
     for (auto xid : conf.i2c) {
          switch(xid) {

                case 0x76:
                sendEvent("Found BME280 sensor at address : 0x"+String(xid,16));
                doshell("bme");
                sendEvent("Sensor inited...");
                addSensor("bmex2tx",1,std::bind(getBME,1),false,pt); 
                addSensor("bmex2hx",1,std::bind(getBME,2),false,pt); 
                addSensor("bmex2px",1,std::bind(getBME,3),false,pt); 
                break;
          }
        vTaskDelay(50);
       sensloaded=true;

      }
      sendEvent("added "+String(pt-1)+" sensors");
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

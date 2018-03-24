using namespace std;
using namespace std::placeholders;

const char mjan[] PROGMEM = "Jan";const char mfeb[] PROGMEM = "Feb";const char mmar[] PROGMEM = "Mar";const char mapr[] PROGMEM = "Apr";const char mmay[] PROGMEM = "May";const char mjun[] PROGMEM = "Jun";const char mjul[] PROGMEM = "Jul";const char maug[] PROGMEM = "Aug";const char msep[] PROGMEM = "Sep";const char moct[] PROGMEM = "Oct";const char mnov[] PROGMEM = "Nov";const char mdec[] PROGMEM = "Dec";
const char * const  xmonth[] PROGMEM = {mjan,mfeb,mmar,mapr,mmay,mjun,mjul,maug,msep,moct,mnov,mdec};

uint8_t gmonth(char * fmonth) {
  for (uint8_t i=0;i<12;i++) if (strcmp_P(fmonth,xmonth[i])==0) return i+1;
}


void ctime(char * fdate) {
  std::vector<String>xpar = ssplit(fdate," ");
  uint8_t sdate,smon,shour,smin,ssec; int syear;
  for (int i=0;i<xpar.size();i++) {
    switch(i) {
      case 1:sdate = xpar[i].toInt();break;
      case 2: smon =  gmonth(&xpar[i][0]);break;      
      case 3: syear = xpar[i].toInt();break;
      case 4: { 
        shour = xpar[i].substring(0,2).toInt();
        smin  = xpar[i].substring(3,5).toInt();
        ssec  = xpar[i].substring(6,8).toInt();
      }
     break;
    }
}

 setTime(shour,smin,ssec,sdate,smon,syear);
 conf.startTime = now();
 //setTime(now()+3600*conf.timeZone);
 
//  sendEvent(String(sdate)+" - "+String(smon)+" - "+String(syear)+" - "+String(shour)+" - "+String(smin)+" - "+String(ssec),"sms");  
}

#define countof(a) (sizeof(a) / sizeof(a[0]))


String secTime(int stime) {
int hours=0, days=0, xminutes=0, sminutes=0, shour=0, xseconds = 0; 
  days = stime/86400;
  if (days) {
  hours = stime-days*86400;
  } else hours = stime;
   shour = hours/3600;
  if (shour) {
   xminutes = hours-shour*3600;
  } else xminutes = stime;
    sminutes = xminutes/60;
  if (sminutes) {
   xseconds = xminutes-sminutes*60;
  } else xseconds = stime;
 
  char buff[30];
      snprintf_P(buff, 
            countof(buff),
            PSTR("%03u %02u:%02u:%02u"),
            days,
            shour,
            sminutes,
            xseconds
            );
    return String(buff);
}

String getUTime() {

//    RtcDateTime now = Rtc.GetDateTime();
//     RtcTemperature temp = Rtc.GetTemperature();
//    dtemp = temp.AsFloat();
    return printDateTime();
}

String printDateTimeT(long t) {
    char datestring[30];

    snprintf_P(datestring, 
            countof(datestring),
            PSTR("%02u/%02u/%02u %02u:%02u:%02u"),
            year(t),            
            month(t),
            day(t),
            hour(t),
            minute(t),
            second(t));
    return String(datestring);
}

String printDateTime() {
    char datestring[30];
  time_t t = now();//+3600*conf.timeZone;
    snprintf_P(datestring, 
            countof(datestring),
            PSTR("%02u/%02u/%02u %02u:%02u:%02u"),
            year(t),            
            month(t),
            day(t),
            hour(t),
            minute(t),
            second(t));
    return String(datestring);
}


String printDate() {
    char datestring[14];

    snprintf_P(datestring, 
            countof(datestring),
            PSTR("%02u-%02u-%02u"),
            year(),            
            month(),
            day());
    return String(datestring);
}


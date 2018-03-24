using namespace std;
using namespace std::placeholders;

WiFiUDP Udp;
unsigned int localPort = 8888;  // local port to listen for UDP packets
static const char ntp1[] = "ntp1.colocall.net";
static const char ntp2[] = "pool.ntp.org";
static const char ntp3[] = "1.pool.ntp.org";

static const char * const nptservs[] = {ntp1,ntp2,ntp3};
//static const char ntpServerName[] = "us.pool.ntp.org";

const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets
uint8_t ntpid = 0;

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}

//time_t savedtime = 0;
 
time_t getNtpTime()
{

//  time_t xnow = now();if (xnow>1000000000) savedTime = xnow+7;
  IPAddress ntpServerIP; // NTP server's ip address

  while (Udp.parsePacket() > 0) ; // discard any previously received packets
//  Serial.println("Transmit NTP Request");
//  sendEvent("Transmit NTP Request","sms");
  // get a random server from the pool
  char ntps[20]; 
  strcpy(ntps,nptservs[ntpid]);
  WiFi.hostByName(ntps, ntpServerIP);
  Serial.print(ntps);
  Serial.print(": ");
  Serial.println(ntpServerIP);
  sendEvent(String(ntps)+": "+String(ntpServerIP),"sms");
  sendEvent("Get NTP time : "+String(ntps),"sms");
  sendNTPpacket(ntpServerIP);  
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      time_t xnn = secsSince1900 - 2208988800UL;
      if (xnn>1400000000 && xnn<2400000000) conf.startTime = xnn;
//      !conf.startTime && 
//      Serial.println(printDateTimeT(xnn));
      sendEvent(printDateTimeT(xnn),"sms");
      setTime(xnn);
      return xnn;
      
      //+ conf.timeZone * SECS_PER_HOUR;
    }
  }
  Serial.println("No NTP Response :-(");
    sendEvent("No NTP Response :-(","sms");
    ntpid++; if (ntpid>3) ntpid=0;
  
  //if (!conf.startTime) return savedTime+7; else 
  return 0; // return 0 if unable to get the time
}


//time_t getNtpTime();
//void digitalClockDisplay();
//void printDigits(int digits);
//void sendNTPpacket(IPAddress &address);

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
  time_t t = now()+3600*conf.timeZone;
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


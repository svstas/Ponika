void sendEvent(String zxmsg, const char* tp) {
    events.send(zxmsg.c_str(),tp);
}

void charEvent(const char* zxmsg) {
    events.send(zxmsg,"sms");
}

void charEventL(const char* zxmsg,char* next) {
    strcpy(next,"!*!");
    strcpy(next,zxmsg);
    events.send(next,"sl");
}

void charEventR(const char* zxmsg) {
    events.send(zxmsg,"r");
}

String printAddress(DeviceAddress deviceAddress)
{
  String out="";
  for (uint8_t i = 0; i < 8; i++)
  {
    // zero pad the address if necessary
    if (deviceAddress[i] < 16) out+="0";
    out+=String(deviceAddress[i],HEX);
  }
  return out;
}




using namespace std;
using namespace std::placeholders;


   // This sends the updated pixel color to the ha

void ow() {

String out = "Dallas temps ";
out +=String(dsensors.getDeviceCount()+1,DEC)+"\n";

DeviceAddress dallas;

for (int i=0;i<16;i++) {
  dsensors.getAddress(dallas, i);
//  out += "Sensor"
if (dsensors.getAddress(dallas, i)) out+="Sensor "+String(i)+" : ds"+printAddress(dallas)+"\n";
}
//  if (!sensors.getAddress(insideThermometer, 0)) Serial.println("Unable to find address for Device 0"); 

sendEvent(out,"sms");
  
}



void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){

  if(type == WS_EVT_CONNECT){
    Serial.printf("ws[%s][%u] connect\n", server->url(), client->id());
    client->printf("Hello Client %u :)", client->id());
    client->ping();
  } else if(type == WS_EVT_DISCONNECT){
    Serial.printf("ws[%s][%u] disconnect: %u\n", server->url(), client->id());
  } else if(type == WS_EVT_ERROR){
    Serial.printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t*)arg), (char*)data);
  } else if(type == WS_EVT_PONG){
    Serial.printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len)?(char*)data:"");
  } else if(type == WS_EVT_DATA){
    AwsFrameInfo * info = (AwsFrameInfo*)arg;
    String msg = "";
    if(info->final && info->index == 0 && info->len == len){
      //the whole message is in a single frame and we got all of it's data
      Serial.printf("ws[%s][%u] %s-message[%llu]: ", server->url(), client->id(), (info->opcode == WS_TEXT)?"text":"binary", info->len);

      if(info->opcode == WS_TEXT){
        for(size_t i=0; i < info->len; i++) {
          msg += (char) data[i];
        }
      } else {
        char buff[3];
        for(size_t i=0; i < info->len; i++) {
          sprintf(buff, "%02x ", (uint8_t) data[i]);
          msg += buff ;
        }
      }
      Serial.printf("%s\n",msg.c_str());

      if(info->opcode == WS_TEXT) {

        client->text(String(msg+"\n"));
//        doshell(msg,client);
     if (msg.substring(0,1) == "a") {
          String spin = msg.substring(1);
          sendEvent(spin,"sms");
       }

        String xout = doshell(msg);
        if (xout.length()) sendEvent(xout,"sms");      

    } else client->binary("I got your binary message");
    
    } else {
      //message is comprised of multiple frames or the frame is split qinto multiple packets
      if(info->index == 0){
        if(info->num == 0)
          Serial.printf("ws[%s][%u] %s-message start\n", server->url(), client->id(), (info->message_opcode == WS_TEXT)?"text":"binary");
        Serial.printf("ws[%s][%u] frame[%u] start[%llu]\n", server->url(), client->id(), info->num, info->len);
      }

      Serial.printf("ws[%s][%u] frame[%u] %s[%llu - %llu]: ", server->url(), client->id(), info->num, (info->message_opcode == WS_TEXT)?"text":"binary", info->index, info->index + len);

      if(info->opcode == WS_TEXT){
        for(size_t i=0; i < info->len; i++) {
          msg += (char) data[i];
        }
      } else {
        char buff[3];
        for(size_t i=0; i < info->len; i++) {
          sprintf(buff, "%02x ", (uint8_t) data[i]);
          msg += buff ;
        }
      }

      
      Serial.printf("%s\n",msg.c_str());


      if((info->index + len) == info->len){
        Serial.printf("ws[%s][%u] frame[%u] end[%llu]\n", server->url(), client->id(), info->num, info->len);
        if(info->final){
          Serial.printf("ws[%s][%u] %s-message end\n", server->url(), client->id(), (info->message_opcode == WS_TEXT)?"text":"binary");
          if(info->message_opcode == WS_TEXT)
            client->text("I got your text message2");
          else
            client->binary("I got your binary message");
        }
      }
    }
  }
}


//SPIFFEDITOR;
class SPIFFSEditor: public AsyncWebHandler {
  
  private:
    String _username;
    String _password;
    bool _authenticated;
    uint32_t _startTime;
  public:
    SPIFFSEditor(String username=String(), String password=String()):_username(username),_password(password),_authenticated(false),_startTime(0){}
    bool canHandle(AsyncWebServerRequest *request){
      if(request->url().equalsIgnoreCase("/admin/edit/")){
        if(request->method() == HTTP_GET){
          if(request->hasParam("list"))
            return true;
          if(request->hasParam("setPar"))
            return true;
          if(request->hasParam("edit")){
            request->_tempFile = SPIFFS.open(request->arg("edit"), "r");
            if(!request->_tempFile)
              return false;
          }
          if(request->hasParam("download")){
            request->_tempFile = SPIFFS.open(request->arg("download"), "r");
            if(!request->_tempFile)
              return false;
          }
          return true;
        }
        else if(request->method() == HTTP_POST)
          return true;
        else if(request->method() == HTTP_DELETE)
          return true;
        else if(request->method() == HTTP_PUT)
          return true;

      }
      return false;
    }

    //bool xlock = false;
    void handleRequest(AsyncWebServerRequest *request){
  
      if(_username.length() && _password.length() && !request->authenticate(_username.c_str(), _password.c_str()))
        return request->requestAuthentication();

      if(request->method() == HTTP_GET){
            
       if(request->hasParam("edit") || request->hasParam("download")){
          request->send(request->_tempFile, request->_tempFile.name(), String(), request->hasParam("download"));
        }
        else {

          request->send(SPIFFS, "/admin/edit.html");


        }
      } else if(request->method() == HTTP_DELETE){
       
        if(request->hasParam("path", true)){
          
          SPIFFS.remove("/admin/dirs.ini");
          SPIFFS.remove(request->getParam("path", true)->value());
          request->send(200, "", "DELETE: "+request->getParam("path", true)->value());
          
          //xlock=true;

        } else
          request->send(404);
      } else if(request->method() == HTTP_POST){
        if(request->hasParam("data", true, true) && SPIFFS.exists(request->getParam("data", true, true)->value()))
          request->send(200, "", "UPLOADED: "+request->getParam("data", true, true)->value());
        else
          request->send(500);
      } else if(request->method() == HTTP_PUT){
        if(request->hasParam("path", true)){
          String filename = request->getParam("path", true)->value();
  
          ESP.wdtDisable();

          
          if(SPIFFS.exists(filename)){
          
            request->send(200);
            
          } else {
 while(xlock); xlock=true;
 xpix(0,conf.clr[2]);
            fs::File f = SPIFFS.open(filename, "w");
            if(f){
              f.write((uint8_t)0x00);
              f.close();
              SPIFFS.remove("/admin/dirs.ini");
xlock=false; 
xpix(0,conf.clr[0]);
              request->send(200, "", "CREATE: "+filename);
              ESP.wdtEnable(300);
            } else {
              request->send(500);
            }
          }
        } else {
 //         xlock=false;
          request->send(400);
       }
     }
//     xlock=false;
    }

    void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
      if(!index){
        if(!_username.length() || request->authenticate(_username.c_str(),_password.c_str())){
          _authenticated = true;
          request->_tempFile = SPIFFS.open(filename, "w");
          _startTime = millis();
        }
      }
      if(_authenticated && request->_tempFile){
        if(len){
          ESP.wdtDisable();
          while(xlock); xlock=true;
           request->_tempFile.write(data,len);
          xlock=false;
          ESP.wdtEnable(30);
         }
        if(final){
          request->_tempFile.close();
        }
      }
    }
};


#define countof(a) (sizeof(a) / sizeof(a[0]))

void scan_i2c(bool out = true) {  
  byte error, address;
  int nDevices;
  String xout = "\n";

  std::vector<uint16_t>i2c;
 
  nDevices = 0;
  for(address = 1; address < 127; address++ ) 
  {
 
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0)
    {
      xout+="I2C device found at address 0x";
      if (address<16) 

        xout+=("0");
        xout+=String(address,HEX);        
        i2c.push_back(address);
        xout+="\n";
      nDevices++;
    }
    else if (error==4) 
    {
      xout+="Unknow error at address 0x";
      if (address<16) 
        xout+="0";
        xout+=String(address,HEX);
        xout+="\n";
    }    
  }
  if (nDevices == 0)
    xout+="No I2C devices found\n";
  else
    xout+="";
    if (out) sendEvent(xout,"sms");
// return xout;

  //std::vector<uint16_t>(conf.i2c).swap(i2c);
  conf.i2c = i2c;
  std::vector<uint16_t>(i2c).swap(i2c);
}



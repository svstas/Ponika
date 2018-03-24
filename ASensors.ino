
using namespace std;
using namespace std::placeholders;





/*
   
   float   getbmp1t() {return bmp1.readTemperature()+3;}
   int32_t getbmp1p() {return bmp1.readPressure()+5;}
   float   getbmp2t() {return bmp2.readTemperature()+7;}
   int32_t getbmp2p() {return bmp2.readPressure()+9;}



x = x
p = p + q;
k = p / (p + r);
x = x + k * (measurement – x);
p = (1 – k) * p;

Where:
q = process noise covariance
r = measurement noise covariance
x = value of interest
p = estimation error covariance
k = Kalman gain


*/

//BH1750 light1(0x23);
//BH1750 light2(0x5c);
//Adafruit_BME280 BME;


RWS_BMP180 bmp1;

HTU21D htu;

BME280I2C BME(1,1,1);
//if (find_i2c(0x76))  

MCP3421 MCP = MCP3421();

TSL2561 tsl(TSL2561_ADDR_FLOAT);


/*
  BME.settings.commInterface = I2C_MODE;
  BME.settings.I2CAddress = BMEaddr;
  BME.settings.runMode = 1;            //  1, Single mode
  BME.settings.tStandby = 0;           //  0, 0.5ms
  BME.settings.filter = 0;             //  0, filter off
  BME.settings.tempOverSample = 1;
  BME.settings.pressOverSample = 1;
  BME.settings.humidOverSample = 1;  
 
*/

float analog() {return analogRead(0);}


float getDal(uint8_t i) {

//   DeviceAddress dallas;
//   if (!dlslist[i]) return -127;
if (!(dlslist.size()>i)) {sendEvent("DS1820 ERR: "+String(dlslist.size())+':'+String(i),"sms");return -127;}
 //   delay(4); 

       dsensors.requestTemperatures();//ByAddress(dlslist[i].addr); 

    if (dsensors.isConversionAvailable(dlslist[i].addr)) {
//    if (dsensors.isConversionComplete()) {

  
      float res = dsensors.getTempC(dlslist[i].addr);
      return res;
    }
    return -127;
}

void loadSensors() {

Serial.println("Start sensors load");


  dsensors.setWaitForConversion(false);
  dsensors.requestTemperatures();


        uint8_t i = 0;
  DeviceAddress dallas;

    while (dsensors.getAddress(dallas, i)) {
                
                char dname[18] = "ds";
                dallasCount++;
                strcat(dname,printAddress(dallas).c_str()); 
                Serial.println(printAddress(dallas));
                mysensors->addSensor(dname,1,std::bind(getDal,i),false,(50+i));
                dsensors.getAddress(dlslist[i].addr,i);

      i++;
     }
       
      


if (find_i2c(0x77)) {
Serial.println("MS5611 found...");
/*
MS5611 xms5611;


  // Initialize MS5611 sensor
  // Ultra high resolution: MS5611_ULTRA_HIGH_RES
  // (default) High resolution: MS5611_HIGH_RES
  // Standard: MS5611_STANDARD
  // Low power: MS5611_LOW_POWER
  // Ultra low power: MS5611_ULTRA_LOW_POWER
  
// while(!xms5611.begin(MS5611_ULTRA_HIGH_RES))
 while(!xms5611.begin(MS5611_ULTRA_LOW_POWER))
 {
   Serial.println("Could not find a valid MS5611 sensor, check wiring!");
   delay(500);
}
  Serial.print("Oversampling: ");
  Serial.println(xms5611.getOversampling());

Serial.print("Pressure::");
Serial.println(xms5611.readPressure());
   mysensors->addSensor("xms5611",1,std::bind(&MS5611::readPressure,&xms5611,true),false,15);
   mysensors->addSensor("xts5611",1,std::bind(&MS5611::readTemperature,&xms5611,true),false,16);
*/

}

if (find_i2c(0x68)) {
Serial.println("MCP3421 found...");
 MCP.init(0x68,3,3);
 //vin=MCP.getDouble();
   mysensors->addSensor("mcp3421",1,std::bind(&MCP3421::getLong,&MCP),false,14);

}

//float tems = sht.readTemperature();
//float hums = sht.readHumidity();

//   mysensors->addSensor("shtt",1,std::bind(&SHT3X::readTemperature,&sht),false,1);
//   mysensors->addSensor("shth",1,std::bind(&SHT3X::readHumidity,&sht),false,2);
   

if (find_i2c(0x39)) {


  if (tsl.begin()) {
    Serial.println("Found sensor");
  } else {
    Serial.println("No sensor?");
    while (1);
  }

   tsl.setGain(TSL2561_GAIN_16X);
   tsl.setTiming(TSL2561_INTEGRATIONTIME_13MS);

// TSL2561.init();
//tsl.getLuminosity(TSL2561_VISIBLE); 
//   mysensors->addSensor("tsl2561",1,std::bind(&Digital_Light_TSL2561::readVisibleLux,&TSL2561),false,13);
//   mysensors->addSensor("tsl2561",1,std::bind(&TSL2561::getLuminosity,&tsl,TSL2561_VISIBLE),false,13);
   mysensors->addSensor("tsl2561",1,std::bind(&TSL2561::getLuminosity,&tsl,TSL2561_FULLSPECTRUM),false,13);
//   mysensors->addSensor("tsl2561",1,std::bind(&TSL2561::getLuminosity,&tsl,TSL2561_INFRARED),false,13);


}


htu.begin();
float humd = htu.readHumidity();

  if (humd>500) Serial.println("HTU21 Failed!");
  else {

        Serial.println("HTU21 success!"); 
   sendEvent("[SENS] HTU21 Success!","sms");

   mysensors->addSensor("htuh",1,std::bind(&HTU21D::readHumidity,&htu),false,2);
   mysensors->addSensor("htut",1,std::bind(&HTU21D::readTemperature,&htu),false,1);
      Serial.println("HTU21H:");Serial.println(htu.readHumidity());
      Serial.println("HTU21T:");Serial.println(htu.readTemperature());
  
    
  }


  if (!bmp1.begin(BMP180_ULTRAHIGHRES)) Serial.println("BMP 180 failed!"); else 
  {
    Serial.println("BMP 180 success!"); 
   sendEvent("[SENS] BMP180 Success!","sms");
     mysensors->addSensor("bmp1p",1,std::bind(&RWS_BMP180::readPressure,&bmp1),false,4);
     mysensors->addSensor("bmp1t",1,std::bind(&RWS_BMP180::readTemperature,&bmp1),false,3);
      Serial.println("BMP1P:");Serial.println(bmp1.readPressure());
      Serial.println("BMP1T:");Serial.println(bmp1.readTemperature());

    }


  if (find_i2c(0x76)) {
  


  if (!BME.begin())  Serial.println("BME failed!"); else {

   Serial.println("BME success!");
   sendEvent("[SENS] BME Success!","sms");

     mysensors->addSensor("bme2h",1,std::bind(&BME280I2C::hum,&BME),false,7);   
     mysensors->addSensor("bme2t",1,std::bind(&BME280I2C::temp,&BME,true),false,5);
     mysensors->addSensor("bme2p",1,std::bind(&BME280I2C::pres,&BME,0x0),false,6);     
 

     
     Serial.println("BMEP:");Serial.println(BME.pres());
     Serial.println("BMET:");Serial.println(BME.temp());
     Serial.println("BMEH:");Serial.println(BME.hum());

  }
  }



mysensors->addSensor("anal", 1,std::bind(analog),false,10);    
//delay(500);


if (find_i2c(0x40)) {
// Adafruit_INA219 ina219;
  SDL_Arduino_INA3221 ina;
  ina.begin();
 
sendEvent("Bus Voltage (V)-> "+String(ina.getBusVoltage_V(1)),"sms");
sendEvent("Shunt Voltage (mV)-> "+String(ina.getShuntVoltage_mV(1)),"sms");
sendEvent("Curent (mA)-> "+String(ina.getCurrent_mA(1)),"sms");

   mysensors->addSensor("inabv",1,std::bind(&SDL_Arduino_INA3221::getBusVoltage_V,&ina,1),false,8);   
   mysensors->addSensor("inama",1,std::bind(&SDL_Arduino_INA3221::getCurrent_mA,&ina,1),false,9);   
//   mysensors->addSensor("inasv",1,std::bind(&SDL_Arduino_INA3221::getShuntVoltage_mV,&ina,1),false,10);        

}



/*  

  if (!BME.begin())  Serial.println("BME failed!"); else {
   Serial.println("BME success!");
   sendEvent("[SENS] BME Success!","sms");
     mysensors->addSensor("bme2p",1,std::bind(&Adafruit_BME280::readPressure,&BME),false,6);
     mysensors->addSensor("bme2t",1,std::bind(&Adafruit_BME280::readTemperature,&BME),false,5);
     mysensors->addSensor("bme2h",1,std::bind(&Adafruit_BME280::readHumidity,&BME),false,7);
     
     Serial.println("BMP2P:");Serial.println(BME.readPressure());
     Serial.println("BMP2T:");Serial.println(BME.readTemperature());
   
  }  

*/

/*
  light1.begin(BH1750_CONTINUOUS_HIGH_RES_MODE);
  int16_t lg = light1.readLightLevel();
   if (lg==54612 || lg<0)   sendEvent("[SENS] BH17501 Failed!","sms"); else {

  sendEvent("[SENS] BH1750 Success!","sms");
  mysensors->addSensorI("light1",1,std::bind(&BH1750::readLightLevel,&light1),true,7);
  Serial.println("BH17501:");Serial.println(light1.readLightLevel());
   }

  light2.begin(BH1750_CONTINUOUS_HIGH_RES_MODE);
   lg = light2.readLightLevel();
   if (lg==54612 || lg<0)   sendEvent("[SENS] BH17502 Failed!","sms"); else {

  sendEvent("[SENS] BH1750 Success!","sms");
  mysensors->addSensorI("light2",1,std::bind(&BH1750::readLightLevel,&light2),true,8);
  Serial.println("BH17502:");Serial.println(light2.readLightLevel());
   }

*/


    mysensors->xinit();
    Serial.println(now());
}

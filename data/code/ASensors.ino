
using namespace std;
using namespace std::placeholders;




/*
   
   float   getbmp1t() {return bmp1.readTemperature()+3;}
   int32_t getbmp1p() {return bmp1.readPressure()+5;}
   float   getbmp2t() {return bmp2.readTemperature()+7;}
   int32_t getbmp2p() {return bmp2.readPressure()+9;}

*/



//BH1750 light1(0x23);
//BH1750 light2(0x5c);
//Adafruit_BME280 BME;

RWS_BMP180 bmp1;
HTU21D htu;
BME280I2C BME(1,1,1); 


Adafruit_INA219 ina219;





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

void loadSensors() {


  



     for (int i=0;i<16;i++) {
   
          DeviceAddress dallas;

      if (dsensors.getAddress(dallas, i)) 
              {
                char dname[18] = "ds";
                dallasCount++;
                strcat(dname,printAddress(dallas).c_str()); 
                mysensors->addSensor(dname,1,std::bind(&DallasTemperature::getTempCByIndex, &dsensors, i),false,(50+i));
 //               sendEvent("[SENS] Dallas:"+printAddress(dallas),"sms");
 
               Serial.print("Dallas :");
               Serial.print(printAddress(dallas));
               Serial.print("; Temp :");
               
               Serial.println(dsensors.getTempCByIndex(i));
                                 
              }
       
      }


//float tems = sht.readTemperature();
//float hums = sht.readHumidity();

//   mysensors->addSensor("shtt",1,std::bind(&SHT3X::readTemperature,&sht),false,1);
//   mysensors->addSensor("shth",1,std::bind(&SHT3X::readHumidity,&sht),false,2);
   



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


mysensors->addSensor("anal", 1,std::bind(&BME280I2C::anal,&BME),false,10);    
//delay(500);


if (find_i2c(0x40)) {
  ina219.begin();
}
 


 
 
sendEvent("Bus Voltage (V)-> "+String(ina219.getBusVoltage_V()),"sms");
sendEvent("Shunt Voltage (mV)-> "+String(ina219.getShuntVoltage_mV()),"sms");
sendEvent("Curent (mA)-> "+String(ina219.getCurrent_mA()),"sms");
// shuntvoltage = ina219.getShuntVoltage_mV();
//  busvoltage = ina219.getBusVoltage_V();
//  current_mA = ina219.getCurrent_mA();

  //ina219.setCalibration_16V_400mA();



   mysensors->addSensor("inabv",1,std::bind(&Adafruit_INA219::getBusVoltage_V,&ina219),false,8);   
   mysensors->addSensor("inama",1,std::bind(&Adafruit_INA219::getCurrent_mA,&ina219),false,9);   
//   mysensors->addSensor("inasv",1,std::bind(&Adafruit_INA219::getShuntVoltage_mV,&ina219),false,10);        


//   mysensors->addSensor("analog",1,std::bind(analogRead(0)),false,10);   
  
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

}

TaskHandle_t adcHandler = NULL;
#include <i2s.h>

void dotask(bool flag) {
String xt;
if (flag) {xt = tasks.back();tasks.pop_back();}
if (xt=="adc") {if (adcHandler == NULL) {
    xTaskCreatePinnedToCore(i2s_adc_task, "adc", 8192, NULL, 1, &adcHandler,1);
    sendEvent("Mic started.");
} else {
    sendEvent("adcoff","c");
    vTaskDelete(adcHandler);isadc=false;
    vTaskDelay(100);
    adcHandler = NULL;
    i2s_off();
    sendEvent("Mic stopped.");
}}
//if (xt=="goff") if (GestHandler != NULL) {vTaskDelete(GestHandler);isgest=false;sendEvent("GESTURE: OFF");}

//if (xt=="gest") {
//if (!isgest) {sendEvent("GESTURE: ON");xTaskCreatePinnedToCore(gesture, "gesture", 8192, NULL, 2, &GestHandler,1);isgest=true;}
////else {vTaskDelete(&GestHandler);isgest = false;sendEvent("GESTURE: OFF");}
//}
//#endif

if (xt=="i2c") { scan_i2c();return;} 

//if (xt=="i2s") i2sInit();
//if (xt=="lora") LoraInit();

}

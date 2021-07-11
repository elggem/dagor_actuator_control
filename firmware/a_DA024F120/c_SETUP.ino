
static const char* ssid = "Spener";
static const char* password = "MoMaCaLa";
String version = "DAGOR-DEV-v15-STATUSA"; 

#include "driver/mcpwm.h"
#include "soc/mcpwm_reg.h"
#include "soc/mcpwm_struct.h"

//###########################################
// SETUP
//###########################################

TaskHandle_t BroadcastStatusTask;


void setup() {
  Serial.begin(115200);
  Serial.println("DAGOR: INIT");
  Serial.println(version);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  int counter = 0;
  while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      delay(500);
      counter++;
      if(counter>=4) ESP.restart();
  }

  Serial.print("\n ESP Board MAC Address:  ");
  Serial.println(WiFi.macAddress());

  WiFiClient client;

  t_httpUpdate_return ret = httpUpdate.update(client, "192.168.178.158", 9000, "/update.php", version);
  switch(ret) {
      case HTTP_UPDATE_FAILED:
          Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
          break;
      case HTTP_UPDATE_NO_UPDATES:
          Serial.println("[update] Update no Update.");
          break;
      case HTTP_UPDATE_OK:
          Serial.println("[update] Update ok."); // may not called we reboot the ESP
          break;
  }

  WiFi.disconnect(true);

  preferences.begin("dagor", false);

  // calibration
  skipCalibration = preferences.getBool("skipCalibration", false); // Skip the calibration on start-up
  elecOffset      = preferences.getFloat("elecOffset", 0.0); // Printed as: "MOT: Zero elec. angle: X.XX"
  natDirection = preferences.getString("natDirection", "CW"); // Can be either CW or CCW     
  sensorOffset = preferences.getFloat("sensorOffset", 0.0);

  Serial.printf("elecOffset: %.2f\n", elecOffset);
  Serial.printf("sensorOffset: %.2f\n", sensorOffset);

  espNowInit(); 

  gpio_init();
  spi_init();
  delay(250);
  drv_init();


  SimpleFOCinit();

  xTaskCreatePinnedToCore(
        espNowBroadcastStatus, /* Function to implement the task */
        "BroadcastStatus", /* Name of the task */
        2048,  /* Stack size in words */
        NULL,  /* Task input parameter */
        0,  /* Priority of the task */
        &BroadcastStatusTask,  /* Task handle. */
        0); /* Core where the task should run */
}

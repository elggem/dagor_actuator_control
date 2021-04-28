
static const char* ssid = "Spener";
static const char* password = "MoMaCaLa";
String version = "DAGOR-DEV-v10"; 

//###########################################
// SETUP
//###########################################

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
  espNowInit();

  gpio_init();
  spi_init();
  delay(250);
  drv_init();

  SimpleFOCinit();
  
}

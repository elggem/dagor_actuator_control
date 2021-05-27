
#define TRY_ESP_ACTION(action, name) if(action == ESP_OK) {Serial.println(String(name));} else {Serial.println("----------Error while " + String(name) + " !---------------");}

#define CHANNEL 1
#define DATARATE WIFI_PHY_RATE_24M

//######_ESPNOW_######
typedef struct struct_message {
    char motor_id = 'M';
    char function = '\0';
    float value;
} struct_message;

typedef struct struct_status_message {
    float position = 0.0;
    float temperature = 0.0;
    float current_d = 0.0;
    float current_q = 0.0;
} struct_status_message;

struct_message inputData;
struct_status_message outputData;
esp_now_peer_info_t peerInfo;


// Master Controller Address
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t master_mac[] = {0xE8, 0x94, 0xF6, 0x27, 0xD1, 0xE6};

void espNowBroadcastStatus(void *pvParameter){

  while (true) {
    //Serial.print("espNowBroadcastStatus() running on core ");
    //Serial.println(xPortGetCoreID());
    
    outputData.position = sensor.getAngle();
    
    float vOut = analogRead(vTemp);
    outputData.temperature = (((vOut*3.3)/4095)-1.8577)/-0.01177;
    
    float ea = motor.electricalAngle();
    DQCurrent_s current = current_sense.getFOCCurrents(ea);
    outputData.current_d = current.d;
    outputData.current_q = current.q;
    
    esp_err_t result = esp_now_send(master_mac, (uint8_t *) &outputData, sizeof(outputData));
  
    //Serial.println("Sent status package");

    vTaskDelay(200 / portTICK_PERIOD_MS);
  }
}


void espNowInit(){
  // Init ESP-NOW
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);

  TRY_ESP_ACTION( esp_wifi_stop(), "stop WIFI");
  TRY_ESP_ACTION( esp_wifi_deinit(), "De init");
  wifi_init_config_t my_config = WIFI_INIT_CONFIG_DEFAULT();
  my_config.ampdu_tx_enable = 0;
  TRY_ESP_ACTION( esp_wifi_init(&my_config), "Disable AMPDU");
  TRY_ESP_ACTION( esp_wifi_start(), "Restart WiFi");
  TRY_ESP_ACTION( esp_wifi_set_promiscuous(true), "Set promiscuous");
  TRY_ESP_ACTION( esp_wifi_set_channel(CHANNEL, WIFI_SECOND_CHAN_NONE), "Set channel");
  TRY_ESP_ACTION( esp_wifi_internal_set_fix_rate(ESP_IF_WIFI_STA, true, DATARATE), "Fixed rate set up");
  TRY_ESP_ACTION( esp_now_init(), "ESPNow Init");
  
  esp_now_register_recv_cb(OnDataRecv);
  esp_now_register_send_cb(OnDataSent);

  // Register peer
  //esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, master_mac, 6);
  peerInfo.channel = CHANNEL;  
  peerInfo.encrypt = false;

  // Add peer
  TRY_ESP_ACTION( esp_now_add_peer(&peerInfo), "Add peer");

  // Send few packages to activate radio/adc 
  for (int i=0;i<10;i++) {
      //Serial.println("sending package");
      esp_err_t result = esp_now_send(master_mac, (uint8_t *) &outputData, sizeof(outputData));
      int foo = analogRead(35);
      delay(10);
  }
  
}

//void espNowStartSend() {
//  timer = timerBegin(3, 80, true);
//  timerAttachInterrupt(timer, &sendStatusData, true);
//  timerAlarmWrite(timer, 1000000, true); //1second
//  timerAlarmEnable(timer);
//}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {

  if (compareMacs(master_mac, mac)) {
      //Serial.println("received package from MASTER");

      memcpy(&inputData, incomingData, sizeof(inputData));

      String espNowInput = String(inputData.motor_id) + String(inputData.function) + String(inputData.value,3);
      //Serial.println("Received ESPNOW Command: " + espNowInput);
    
      char wirelessCommand[10]; 
      espNowInput.toCharArray(wirelessCommand, sizeof(wirelessCommand));
      commandEspNow.run(wirelessCommand);

      esp_err_t result = esp_now_send(master_mac, (uint8_t *) &outputData, sizeof(outputData));
  } else {
    Serial.println("received package from not master");
  }
  

}

boolean compareMacs(const uint8_t * arrayA, const uint8_t * arrayB) {
    int numItems = 6;
    boolean same = true;
    long i = 0;
    while(i<numItems && same) { 
      same = arrayA[i] == arrayB[i];
      i++;
    }
    return same;
  }


// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
//  Serial.print("Send Status:\t");
//  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

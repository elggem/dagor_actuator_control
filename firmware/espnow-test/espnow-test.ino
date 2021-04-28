#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi_internal.h>

//######_ESPNOW_######
typedef struct struct_message {
    char motor_id = 'M';
    char function = '\0';
    float value;
} struct_message;

struct_message inputData;
struct_message outputData;
esp_now_peer_info_t peerInfo;

// Master Controller Address
//uint8_t broadcastAddress[] = {0xE8, 0x94, 0xF6, 0x27, 0xD1, 0xE6};
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

//------------------------------------------------------------------------------------------

void setup() {
  Serial.begin(115200);
  Serial.println("Hello, this is ESPNOW Test Code.");
  espNowInit();

}

void loop() {

//  sendData();
//  delay(10);
}


//------------------------------------------------------------------------------------------
#define TRY_ESP_ACTION(action, name) if(action == ESP_OK) {Serial.println("\t+ "+String(name));} else {Serial.println("----------Error while " + String(name) + " !---------------");}

#define CHANNEL 1
#define DATARATE WIFI_PHY_RATE_24M

void espNowInit(){
  // Init ESP-NOW
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);

  Serial.println(WiFi.macAddress());

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
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = CHANNEL;  
  peerInfo.encrypt = false;

  // Add peer
  TRY_ESP_ACTION( esp_now_add_peer(&peerInfo), "Add peer");
}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&inputData, incomingData, sizeof(inputData));
//  Serial.print("Bytes received: ");
//  Serial.println(len);
//  Serial.print("Motor ID: ");
//  Serial.println(inputData.motor_id);
//  Serial.print("Function: ");
//  Serial.println(inputData.function);
//  Serial.print("Value: ");
//  Serial.println(inputData.value);

  String espNowInput = String(inputData.motor_id) + String(inputData.function) + String(inputData.value,3);
//  char wirelessCommand[10]; 
//  espNowInput.toCharArray(wirelessCommand, sizeof(wirelessCommand));
  Serial.println(espNowInput);

//  if (inputData.function == "pos"){
//    sendData();
//  }
//  else if(inputData.value == 0){
//    String espNowInput = inputData.function;
//    char wirelessCommand[10]; 
//    espNowInput.toCharArray(wirelessCommand, sizeof(wirelessCommand));
//    //commandEspNow.run(wirelessCommand);
//    Serial.println(wirelessCommand);
//  }
//  else{
//    String espNowInput = inputData.function + String(inputData.value,3);
//    char wirelessCommand[10]; 
//    espNowInput.toCharArray(wirelessCommand, sizeof(wirelessCommand));
//    //commandEspNow.run(wirelessCommand);
//    Serial.println(wirelessCommand);
//  }
  
}

void sendData(){
//  Serial.println("Sending some data");
  outputData.motor_id = 'M';
//  outputData.function = '\0';
  outputData.value = 42.0;
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &outputData, sizeof(outputData));
//  Serial.println(result);
}

// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
//  Serial.print("Send Status:\t");
//  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  /*if (status == 0){
    String success = "Delivery Success :)";
  }
  else{
    String success = "Delivery Fail :(";
  }*/
}

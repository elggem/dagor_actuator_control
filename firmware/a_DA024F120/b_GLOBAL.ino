
//###########################################
// GLOBAL TAB
//###########################################

// Libraries, pin number assignment and instance initialization

//SimpleFOC Version 2.1
#include <SimpleFOC.h>
#include <SPI.h>
#include <esp_now.h>
#include <esp_wifi_internal.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <Preferences.h>

// calibration preferences.h to store values
Preferences preferences;


//#######_THREE PHASE DRIVER - DRV8305_########
// Datasheet: www.ti.com/lit/ds/symlink/drv8305.pdf
#define enGate 2       //Chip Enable
#define nFault 4       //Fault reading
#define cs 5           //DRV8305 Chip-select
#define so1 36
#define so2 35
#define so3 34
bool faultTrig = false;

//######_MAGNETIC SENSOR - AS5147_######
// Datasheet: https://ams.com/documents/20143/36005/AS5147_DS000307_2-00.pdf
#define sensorCS 16             //AS5147 Chip-select

//######_TEMPERATURE SENSOR - STLM20_######
// Datasheet: https://datasheet.lcsc.com/szlcsc/1810010411_STMicroelectronics-STLM20W87F_C129796.pdf
#define vTemp 39

//######_Voltage Monitor_#######
#define vMonitor 33

//#####_TIME MANAGEMENT_#####
unsigned long runTime, prevT = 0, timeDif, stateT;
int timeInterval = 1000, totalTempTime;

//####_SIMPLEFOC INSTANCES_####
BLDCMotor motor = BLDCMotor(pp);                                                      //BLDCMotor instance
BLDCDriver3PWM driver = BLDCDriver3PWM(25, 26, 27);                                   //3PWM Driver instance
MagneticSensorSPI sensor = MagneticSensorSPI(AS5147_SPI, sensorCS);                   //SPI Magnetic sensor instance
LowsideCurrentSense current_sense = LowsideCurrentSense(0.002, 20.0, so1, so2);  //Current sensing instance

//####_COMMANDER INTERFACE_####
Commander command = Commander(Serial);
void onMotor(char* cmd){ command.motor(&motor, cmd); }
Commander commandEspNow = Commander();
void onMotorEspNow(char* cmd){ commandEspNow.motor(&motor, cmd); }

//######_ESPNOW_######
typedef struct struct_message {
    char motor_id = 'M';
    char function = '\0';
    float value;
} struct_message;

typedef struct struct_status_message {
    float position = 0.0;
    float temperature = 0.0;
    float current_a = 0.0;
    float current_b = 0.0;
} struct_status_message;

struct_message inputData;
struct_status_message outputData;
esp_now_peer_info_t peerInfo;


//######_SETUP FUNCTIONS INIT_######
void SimpleFOCinit();
void drv_init();
void spi_init();
void gpio_init();
void current_dc_calib(bool activate);

//######_LOOP FUNCTIONS INIT_######
void timeManagement();
void tempStatus(bool debug = false);
void voltageMonitor(bool debug = false);
void rotorPosition();
void faultStatus();
void printCurrents(bool dcEquivalent = false);

//######_ESP_NOW_######
void espNowInit();
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len);
void sendData();
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);

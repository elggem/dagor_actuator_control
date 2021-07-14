/*
TBD

Etienne Arlaud
*/

#include <stdint.h>
#include <stdio.h>
#include <string>

#include <assert.h>
#include <unistd.h>
#include <sys/time.h>

#include <thread>
#include <math.h>

#include "ESPNOW_manager.h"
#include "ESPNOW_types.h"

using namespace std;

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

struct_status_message inputData;
struct_message outputData;

static uint8_t my_mac[6] = {0xE8, 0x94, 0xF6, 0x27, 0xD1, 0xE6};
static uint8_t broadcast_mac[6] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

static uint8_t A_mac[6] = {0x8C, 0xCE, 0x4E, 0x8E, 0x2A, 0x50};
static uint8_t B_mac[6] = {0x8C, 0xCE, 0x4E, 0x8E, 0xDC, 0x30};
static uint8_t C_mac[6] = {0x8C, 0xCE, 0x4E, 0x92, 0xC6, 0x4C};
static uint8_t D_mac[6] = {0x8C, 0xCE, 0x4E, 0x8E, 0x2A, 0x58};
static uint8_t E_mac[6] = {0x8C, 0xCE, 0x4E, 0x8E, 0x32, 0x88};
static uint8_t F_mac[6] = {0x8C, 0xCE, 0x4E, 0x8E, 0x3D, 0xFC};

ESPNOW_manager *handler;

void setControlMode(uint8_t dst_mac[6], uint8_t mode) {
    outputData.motor_id = 'M'; // default motor ID
    outputData.function = 'C'; // select control mode
    outputData.value = mode;   // 0=torque, 1=velocity, 2=position
    printf("Sending %c%c%d\n", outputData.motor_id, outputData.function, mode);
    memcpy(handler->mypacket.wlan.actionframe.content.payload, &outputData, 8);
    handler->mypacket.wlan.actionframe.content.length = 8+5;
    handler->set_dst_mac(dst_mac);
    handler->send();
}

void setValue(uint8_t dst_mac[6], float value) {
    outputData.motor_id = 'M'; // default motor ID
    outputData.function = '\0'; // select control mode
    outputData.value = value;   // 0=torque, 1=velocity, 2=position
    printf("Sending %c%c%.8f\n", outputData.motor_id, outputData.function, value);
    memcpy(handler->mypacket.wlan.actionframe.content.payload, &outputData, 8);
    handler->mypacket.wlan.actionframe.content.length = 8+5;
    handler->set_dst_mac(dst_mac);
    handler->send();
}

bool compare_macs(uint8_t src_mac[6], uint8_t target_mac[6]) {
    return !memcmp(target_mac, src_mac, sizeof(uint8_t)*6);
}

void receive_callback(uint8_t src_mac[6], uint8_t *incomingData, int len) {
    //A->B
    //E->D
    //C->F

    if (compare_macs(src_mac, A_mac)) {
      memcpy(&inputData, incomingData, sizeof(inputData));
      setValue(B_mac, -inputData.position);
    }

    if (compare_macs(src_mac, E_mac)) {
      memcpy(&inputData, incomingData, sizeof(inputData));
      setValue(D_mac, -inputData.position);
    }

    if (compare_macs(src_mac, C_mac)) {
      memcpy(&inputData, incomingData, sizeof(inputData));
      setValue(F_mac, inputData.position);
    }
}

int main(int argc, char **argv) {
	assert(argc > 1);

	nice(-20);

	handler = new ESPNOW_manager(argv[1], DATARATE_24Mbps, CHANNEL_freq_1, my_mac, broadcast_mac, false);
	handler->set_recv_callback(&receive_callback);
	handler->start();

	for (int i=0;i<5;i++) {
          setControlMode(A_mac, 0); //torque
          setControlMode(B_mac, 2);
          setControlMode(C_mac, 0);
          setControlMode(D_mac, 2);
          setControlMode(E_mac, 0);
          setControlMode(F_mac, 2);
       }

       while(1) {	
         usleep(10000); //1000hz
       }

       handler->end();
}



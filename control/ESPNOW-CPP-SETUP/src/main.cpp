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

void receive_callback(uint8_t src_mac[6], uint8_t *incomingData, int len) {
    if (memcmp(my_mac, src_mac, sizeof(uint8_t)*6)) {
      memcpy(&inputData, incomingData, sizeof(inputData));
      printf("%x:%x:%x:%x:%x:%x,",src_mac[0],src_mac[1],src_mac[2],src_mac[3],src_mac[4],src_mac[5]); 
      printf("%.8f,%.2f,%.2f,%.2f\n", inputData.position, inputData.temperature, inputData.current_a, inputData.current_b); 
    }
}

void sendReset(uint8_t dst_mac[6]) {
    outputData.motor_id = 'C'; // control
    outputData.function = 'R'; // reset
    outputData.value = 0.0;   // n/a
    printf("Sending %c%c%.2f\n", outputData.motor_id, outputData.function, 0.0);

    memcpy(handler->mypacket.wlan.actionframe.content.payload, &outputData, 8);
    handler->mypacket.wlan.actionframe.content.length = 8+5;
    handler->set_dst_mac(dst_mac);
    handler->send();
}

void sendStore(uint8_t dst_mac[6]) {
    outputData.motor_id = 'C'; // control
    outputData.function = 'S'; // reset
    outputData.value = 0.0;   // n/a
    printf("Sending %c%c%.2f\n", outputData.motor_id, outputData.function, 0.0);

    memcpy(handler->mypacket.wlan.actionframe.content.payload, &outputData, 8);
    handler->mypacket.wlan.actionframe.content.length = 8+5;
    handler->set_dst_mac(dst_mac);
    handler->send();
}

void sendOffset(uint8_t dst_mac[6], float value) {
    outputData.motor_id = 'C'; // control
    outputData.function = 'O'; // offset
    outputData.value = value;   // value
    printf("Sending %c%c%.2f\n", outputData.motor_id, outputData.function, value);

    memcpy(handler->mypacket.wlan.actionframe.content.payload, &outputData, 8);
    handler->mypacket.wlan.actionframe.content.length = 8+5;
    handler->set_dst_mac(dst_mac);
    handler->send();
}

int main(int argc, char **argv) {
	if (argc < 3) {
	  printf("Usage: sudo %s WIFI_NAME TARGET_ID COMMAND (R/S/O) VALUE\n", argv[0]);
	  return 1;
	}

	nice(-20);

	handler = new ESPNOW_manager(argv[1], DATARATE_24Mbps, CHANNEL_freq_1, my_mac, broadcast_mac, false);
	//handler->set_filter(A_mac, my_mac);
        //handler->set_filter(B_mac, my_mac);
        //handler->set_filter(C_mac, my_mac);
        //handler->set_filter(D_mac, my_mac);
        //handler->set_filter(E_mac, my_mac);
        //handler->set_filter(F_mac, my_mac);
	handler->set_recv_callback(&receive_callback);
	handler->start();

	uint8_t *target_mac;

	switch(argv[2][0]) {
	  case 'A':
	    printf("Selecting Module A\n");
            target_mac = A_mac;
	    break;
          case 'B':
            printf("Selecting Module B\n");
            target_mac = B_mac;
            break;
          case 'C':
            printf("Selecting Module C\n");
            target_mac = C_mac;
            break;
          case 'D':
            printf("Selecting Module D\n");
            target_mac = D_mac;
            break;
          case 'E':
            printf("Selecting Module E\n");
            target_mac = E_mac;
            break;
          case 'F':
            printf("Selecting Module F\n");
            target_mac = F_mac;
            break;
          default:
            printf("Usage: sudo %s WIFI_NAME TARGET_ID COMMAND (R/S/O) VALUE\n", argv[0]);
            return 1;
	}

	switch(argv[3][0]) {
	  case 'R':
            sendReset(target_mac);
            break;
          case 'S':
            sendStore(target_mac);
	    break;
          case 'O':
            float value;
            value = atof(argv[4]);
            sendOffset(target_mac, value);
            break;
	}

	printf("Entering monitor mode\n");

	while(1) {
 	  //monitor
          usleep(1000*100);
	}

	handler->end();
}



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

struct_message inputData;
struct_message outputData;

static uint8_t my_mac[6] = {0xE8, 0x94, 0xF6, 0x27, 0xD1, 0xE6};
static uint8_t broadcast_mac[6] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

static uint8_t test_mac[6] = {0x24, 0x0A, 0xC4, 0x60, 0x0B, 0x5C};
static uint8_t leg_mac[6] = {0x8C, 0xCE, 0x4E, 0x8E, 0xDC, 0x30};
static uint8_t hip_mac[6] = {0x8C, 0xCE, 0x4E, 0x8E, 0x32, 0x88};

ESPNOW_manager *handler;

void setControlMode(uint8_t dst_mac[6], uint8_t mode) {
    printf("Sending Control Mode Command\n");

    outputData.motor_id = 'M'; // default motor ID
    outputData.function = 'C'; // select control mode
    outputData.value = mode;   // 0=torque, 1=velocity, 2=position

	memcpy(handler->mypacket.wlan.actionframe.content.payload, &outputData, 8);
	handler->mypacket.wlan.actionframe.content.length = 8+5;
	handler->set_dst_mac(dst_mac);
	handler->send();
}

void setValue(uint8_t dst_mac[6], float value) {
    printf("Sending Value Command\n");

    outputData.motor_id = 'M'; // default motor ID
    outputData.function = '\0'; // select control mode
    outputData.value = value;   // 0=torque, 1=velocity, 2=position

	memcpy(handler->mypacket.wlan.actionframe.content.payload, &outputData, 8);
	handler->mypacket.wlan.actionframe.content.length = 8+5;
	handler->set_dst_mac(dst_mac);
	handler->send();
}

void receive_callback(uint8_t src_mac[6], uint8_t *incomingData, int len) {
	// receive package
    memcpy(&inputData, incomingData, sizeof(inputData));
    printf("Bytes received: %d\n",len);
    printf("Motor ID: %c\n",inputData.motor_id);
    printf("Function: %c\n",inputData.function);
    printf("Value: %.2f\n",inputData.value);
}

int main(int argc, char **argv) {
	assert(argc > 1);

	nice(-20);

	handler = new ESPNOW_manager(argv[1], DATARATE_24Mbps, CHANNEL_freq_1, my_mac, broadcast_mac, false);
	handler->set_filter(test_mac, my_mac);
	handler->set_recv_callback(&receive_callback);
	handler->start();

	usleep(1000*1000);
    setControlMode(leg_mac, 2);
    setControlMode(hip_mac, 2);
	usleep(1000*1000);
    setControlMode(leg_mac, 2);
    setControlMode(hip_mac, 2);

	while(1) {
		// std::this_thread::yield();
		usleep(1000*1000);
        setValue(leg_mac, -5.0);
        setValue(hip_mac, -1.0);
		usleep(1000*1000);
        setValue(leg_mac, 5.0);
        setValue(hip_mac, 1.0);

	}

	handler->end();
}

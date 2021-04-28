/*
TBD

Etienne Arlaud
*/

#include <stdint.h>
#include <stdio.h>

#include <assert.h>
#include <unistd.h>
#include <sys/time.h>

#include <thread>

#include "ESPNOW_manager.h"
#include "ESPNOW_types.h"

using namespace std;

//######_ESPNOW_######
typedef struct struct_message {
    char function;
    float value;
} struct_message;

struct_message inputData;
struct_message outputData;

static uint8_t my_mac[6] = {0xE8, 0x94, 0xF6, 0x27, 0xD1, 0xE6};
static uint8_t dest_mac[6] = {0x24, 0x0A, 0xC4, 0x60, 0x0B, 0x5C};
// static uint8_t broadcastAddress[6] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

ESPNOW_manager *handler;

void callback(uint8_t src_mac[6], uint8_t *incomingData, int len) {
	// receive package
    memcpy(&inputData, incomingData, sizeof(inputData));
    printf("Bytes received: %d\n",len);
    printf("Function: %c\n",inputData.function);
    printf("Value: %.2f\n",inputData.value);

    // send a response
    outputData.function = 'M';
    outputData.value = 2.3;

	memcpy(handler->mypacket.wlan.actionframe.content.payload, &outputData, 8);
	handler->mypacket.wlan.actionframe.content.length = 8+5;
	handler->set_dst_mac(dest_mac);
	handler->send();
}

int main(int argc, char **argv) {
	assert(argc > 1);

	nice(-20);

	handler = new ESPNOW_manager(argv[1], DATARATE_24Mbps, CHANNEL_freq_1, my_mac, dest_mac, false);

	handler->set_filter(dest_mac, my_mac);

	handler->set_recv_callback(&callback);

	handler->start();

	while(1) {
		std::this_thread::yield();
	}

	handler->end();
}

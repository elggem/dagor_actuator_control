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
    printf("Sending %c%c%.3f\n", outputData.motor_id, outputData.function, value);
    memcpy(handler->mypacket.wlan.actionframe.content.payload, &outputData, 8);
    handler->mypacket.wlan.actionframe.content.length = 8+5;
    handler->set_dst_mac(dst_mac);
    handler->send();
}

void receive_callback(uint8_t src_mac[6], uint8_t *incomingData, int len) {
//    memcpy(&inputData, incomingData, sizeof(inputData));
//    printf("%.8f,%.2f,%.2f,%.2f\n", inputData.position, inputData.temperature, inputData.current_a, inputData.current_b);
}

//ik

const int rr = 8;                                       // Reduction ratio of the actuator
const float knee = 200, tibia = 200, yOffset = 65;      // Segment length of the leg parts, yOffset is the offset between the shoulder motor and the axis of rotation

typedef struct Leg{
  int id;
  float theta;
  float phi;
  float gamma;
} Leg;

// Square number funtion
float sq(float numb){
  return numb*numb;
}

// Calculates the angle of the knee actuator as well as part of the angle of the hip actuator
// Takes a pointer to the struct that cointains the angles for the leg
void z(Leg *leg, float height){
  float thetaZ = ( (3.1416/2) - acos( (sq(knee) + sq(height) - sq(tibia)) / ( 2*knee*height ) ) ) * rr;
  float phiZ = ( acos( (sq(knee) + sq(tibia) - sq(height)) / ( 2*knee*tibia ) ) ) * rr;
  float *theta = &(*leg).theta;
  *theta -= thetaZ;
  float *phi = &(*leg).phi;
  *phi = -phiZ;
}

// Calculates part of the angle of the hip actuator as well as the "Leg length" considering the offset cause by inputing a non 0 x coordinate
float x(Leg *leg, float height, float distX){
  float extraTheta = ( atan( distX / height ) );
  float thetaX = extraTheta * rr;
  float newLegLength = ( height / (cos(extraTheta)) );
  //newLegLength = heightRestriction(abs(newLegLength));
  float *theta = &(*leg).theta;
  *theta = thetaX;

  return newLegLength;
}

// Calculates the hip angle
float y(Leg *leg, float height, float posY){
  float distY = yOffset + posY;
  float gammaP = atan( distY / height );
  if (isnan(gammaP)) gammaP = 3.1416/2;
  float hipHyp = distY / sin( gammaP );
  float lambda = asin ( yOffset / hipHyp );
  float gammaY = ( (  - lambda ) + gammaP  ) * rr;
  float newNewLegLength = yOffset/tan(lambda);
  //newNewLegLength = heightRestriction(abs(newNewLegLength));
  float *gamma = &(*leg).gamma;
  *gamma = gammaY;

  return newNewLegLength;
}

void inverseKinematics(Leg *leg, float pos_z, float pos_x, float pos_y){
  //int *id = &(*leg).id;
  //printf("ID: %d \n", (*leg).id);
  //*id = 2;

  z(leg, x(leg, y(leg, pos_z, pos_y), pos_x));

}


int main(int argc, char **argv) {
	assert(argc > 1);

	nice(-20);

	handler = new ESPNOW_manager(argv[1], DATARATE_24Mbps, CHANNEL_freq_1, my_mac, broadcast_mac, false);
	handler->set_recv_callback(&receive_callback);
	handler->start();

	for (int i=0;i<2;i++) {
          setControlMode(A_mac, 2);
          setControlMode(B_mac, 2);
          setControlMode(C_mac, 2);
          setControlMode(D_mac, 2);
          setControlMode(E_mac, 2);
          setControlMode(F_mac, 2);
       }

       //init ik
       Leg legs[] = { {.id = 0}, {.id = 1} };

       int leg_amount = sizeof(legs)/sizeof(legs[0]);

       printf("Amount of legs: %d \n", leg_amount);

       for (int i = 0; i<leg_amount; i++){
         printf("ID: %d, Theta: %f, Phi: %f, Gamma: %f \n", legs[i].id, legs[i].theta, legs[i].phi, legs[i].gamma);
       }

       float z = 150;
       float x = 10;
       float y = -20;

       float speed= 2.25f;
       bool risingZ = true;
       bool risingX = true;
       bool risingY = true;

       // theta, phi, gamma
       // leg1: E/A/C
       // leg2: D/B/F

       while(1) {	
         usleep(1000*10);

         inverseKinematics(&legs[0], z, x, y);
         inverseKinematics(&legs[1], z, x, y);

         //update actuators
         setValue(E_mac, -legs[0].theta);
	 setValue(A_mac, legs[0].phi);
	 setValue(C_mac, legs[0].gamma);
         setValue(D_mac, legs[1].theta);
         setValue(B_mac, -legs[1].phi);
         setValue(F_mac, legs[1].gamma);

         printf("Z: %f, X: %f, Y: %f \n", z,x,y);

      	 for (int i = 0; i<leg_amount; i++){
            printf("ID: %d, Theta: %f, Phi: %f, Gamma: %f \n", legs[i].id, legs[i].theta, legs[i].phi, legs[i].gamma);
         }

//	 break;

	 if(z>150) risingZ = false;
         if(z<10) risingZ = true;
         // going down
	 if(x>10) { risingX = false; if (risingY) y+=20; else y-=20; }
         // going up
	 if(x<-45) { risingX = true; }
	 if(y>160) risingY = false;
         if(y<-60) risingY = true;

	 //if(risingZ) z+=speed; else z-=speed;
         if(risingX) x+=speed; else { x-=speed; }
         //if(risingY) y+=speed; else y-=speed;

       }

       handler->end();
}



/**
 * @file
 * 
 * ESP32 AWS Library
 * 
 * Functions to get the crawler coordinates from the Camera over AWS IoT
 * 
 * Authors: Vipul Deshpande, Jaime Burbano
 */

#ifndef aws_h
#define aws_h

#include <MQTTClient.h>

void messageHandler(String, String);

class myawsclass {
  public:
    myawsclass();

    void connectAWS();                            /* Initialize and connect to AWS */
    void publishMessage(int16_t sensorValue);     /* Publish the values of the sensors */
    // void publishMessage(int16_t sen0, int16_t sen1, int16_t sen2);
    void stayConnected();                         /* Maintain the connection */

    int targetId = -1; // for messageHandler of "tank/target"
};

extern myawsclass awsobject;

#endif
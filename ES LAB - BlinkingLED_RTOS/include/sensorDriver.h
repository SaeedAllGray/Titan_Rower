/**
 * @file
 * 
 * ESP32 Sensor Library
 * 
 * Functions to get distance values from the sensors
 * 
 * Authors: Vipul Deshpande, Jaime Burbano
 */

#ifndef sensor_h
#define sensor_h

class sclass {
  public:
    sclass();
    
    void SETUP();         /* Initialize the Sensor */
    int16_t *reading();   /* Return the values of the sensors as an array */
};

extern sclass sensorobject;
#endif
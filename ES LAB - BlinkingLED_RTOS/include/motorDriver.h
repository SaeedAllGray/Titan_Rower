/**
 * @file
 * 
 * ESP32 Motor Library
 * 
 * Functions to set motor speed
 * 
 * Authors: Vipul Deshpande, Jaime Burbano
 */

#ifndef motor_h
#define motor_h

enum Motors
{
  MotorA = 0,   /* Crawler Motor 1 */
  MotorB = 1    /* Crawler Motor 2 */
};

enum Direction
{
  Forward = 0,  /* Motor Forward */
  Backward = 1  /* Motor Backward */
};

class mclass {
  public:
    mclass();
    
    void SETUP();   /* Initialize the Motors */
    void SPEED(int motor_speed);
    void motor_direction(Motors motor_ch, Direction dir); /* set direction of rotation of the motors */
    void set_speed(Motors motor_ch, Direction dir, int new_speed);  /* set the speed of the motors */
};

extern mclass motorobject;

#endif
/**
 * ESP32 Motor Library
 * 
 * Functions to set motor speed
 * 
 * Authors: Vipul Deshpande, Jaime Burbano
 */

#include "Arduino.h"
#include "motorDriver.h"

static uint8_t inA1 = 16;           /* Pin for Motor 0 */
static uint8_t inA2 = 17;           /* Pin for Motor 0 */
static uint8_t inB1 = 19;           /* Pin for Motor 1 */
static uint8_t inB2 = 33;           /* Pin for Motor 1 */
static int freq = 20000;
static uint8_t Channel1 = 0;
static uint8_t Channel2 = 1;
static uint8_t motorChannel1 = 5;   /* PWM Pin for Motor 0 */
static uint8_t motorChannel2 = 18;  /* PWM Pin for Motor 0 */
static uint8_t resolution = 8;


mclass::mclass() {
}

void mclass::SETUP() {
  pinMode(inA1, OUTPUT);            /* Initializing pins as Outputs */
  pinMode(inA2, OUTPUT);            /* Initializing pins as Outputs */
  pinMode(inB1, OUTPUT);            /* Initializing pins as Outputs */
  pinMode(inB2, OUTPUT);            /* Initializing pins as Outputs */

  ledcSetup(Channel1, freq, resolution);
  ledcAttachPin(motorChannel1, Channel1);

  ledcSetup(Channel2, freq, resolution);
  ledcAttachPin(motorChannel2, Channel2);
}


void mclass::SPEED(int motor_speed) {
  
}

void mclass::motor_direction(Motors motor_ch, Direction dir) {
  
  if (motor_ch == 0)
  {
    if (dir == Forward)
    {
      digitalWrite(inA1,HIGH);  //uncomment to use ESP custom
      digitalWrite(inA2,LOW);   //uncomment to use ESP custom
    }
    else 
    {
      digitalWrite(inA1,LOW);   //uncomment to use ESP custom
      digitalWrite(inA2,HIGH);  //uncomment to use ESP custom
    }
  }

  else
  {
    if (dir == Forward)
    {
      digitalWrite(inB1,LOW);   //uncomment to use ESP custom
      digitalWrite(inB2,HIGH);  //uncomment to use ESP custom
    }
    else
    {
      digitalWrite(inB1,HIGH);  //uncomment to use ESP custom
      digitalWrite(inB2,LOW);   //uncomment to use ESP custom
    }
  }
}

void mclass::set_speed(Motors motor_ch, Direction dir, int new_speed) {
  motor_direction(motor_ch, dir);   /* polarize the motors before setting the speed */
  
  if (new_speed < 0)
  {
    new_speed = 0;
  }

  else if (new_speed > 255)
  {
    new_speed = 255;
  }

  else if (new_speed >= 0 && new_speed <= 255)
  {
    new_speed = new_speed;
  }

  else
  {
    /* Do nothing */
  }

  if (motor_ch == 0)
  {
    ledcWrite(Channel1, new_speed);
  }
  else
  {
    ledcWrite(Channel2, new_speed);
  }
}

mclass motorobject = mclass();  /* creating an object of class motor */

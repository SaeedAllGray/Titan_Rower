#include <Arduino.h>
#include "AWS.h"
#include "motorDriver.h"
#include "sensorDriver.h"

enum class led_mode_t
{
  FAST,
  SLOW
};

const int LED_BOARD = 2; // change here the pin of the board to V2
volatile led_mode_t led_mode = led_mode_t::FAST;
bool led_state = LOW;

void taskOne(void *parameter);

myawsclass aws = myawsclass();
mclass motor = mclass();
sclass sensors = sclass();

void setup()
{
  // ~~I doubt that GPIO2 has been used from interrupt of sensor input, hence should not be used~~
  // maybe not used
  pinMode(LED_BOARD, OUTPUT);

  // For deubg LED
  xTaskCreate(
      taskOne,   /* Task function. */
      "TaskOne", /* String with name of task. */
      /* gonna crash if the stack size is 512 only */
      1024,  /* Stack size in bytes. */
      NULL,  /* Parameter passed as input of the task */
      1,     /* Priority of the task. */
      NULL); /* Task handle. */

  Serial.begin(921600);
  sensors.SETUP();

  // connect Wi-Fi & AWS
  aws.connectAWS();

  // demo motor
  motor.SETUP();
  motor.set_speed(MotorA, Forward, 127);
  motor.set_speed(MotorB, Backward, 127);
  delay(750);
  motor.set_speed(MotorA, Backward, 127);
  motor.set_speed(MotorB, Forward, 127);
  delay(750);
  motor.set_speed(MotorA, Forward, 127);
  motor.set_speed(MotorB, Forward, 127);
  delay(750);
  motor.set_speed(MotorA, Backward, 127);
  motor.set_speed(MotorB, Backward, 127);
  delay(750);
  motor.set_speed(MotorA, Forward, 0);
  motor.set_speed(MotorB, Backward, 0);
}

void loop()
{
  // const int CH_NUM = 3;

  // 0: left, 1: centre, 2: right (on the side with Pololu logo)
  int16_t *value = sensors.reading();

  // (reading() returns value + 100) mm = distance from the edge of the board
  // I don't know why but the value sometimes become around -1000
  Serial.printf("\r% 5d % 5d % 5d", value[0], value[1], value[2]);

  // aws.publishMessage(8964);
  aws.publishMessage(value[0], value[1], value[2]);
  aws.stayConnected();
  delay(1000);
}

void taskOne(void *parameter)
{
  while (true)
  {
    led_state = !led_state;
    digitalWrite(LED_BOARD, led_state);
    int delay_ms = (led_mode == led_mode_t::FAST) ? 100 : 500;

    // delay(1000); //This delay doesn't give a chance to the other tasks to execute
    vTaskDelay(delay_ms / portTICK_PERIOD_MS); // this pauses the task, so others can execute
  }
  vTaskDelete(NULL);
}

#include <Arduino.h>
#include "AWS.h"
#include "environment.h"

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
Environment env = Environment();

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
  env.sensors.SETUP();
  env.motor.SETUP();
  
  // // height: 45 mm
  // // on fully-charged NiMH batteries
  // // motor A (255, Forward):  47     rpm
  // // motor B (255, Backward): 46 1/2 rpm
  // // motor B (127, Backward): 22 1/4 rpm
  // // motor B (63, Backward):  10     rpm
  // // motor B can still spin backwards at speed = 20, but not 15
  // // motor B Backward regression: y = 0.19001x - 1.93499

  // // forward 1m
  // motor.set_speed(MotorB, Backward, 63);
  // motor.set_speed(MotorA, Forward, 63);
  // sleep(10); // 255: 95cm; 127: 43mm; 63: 17.8 mm
  // // // backward 1m
  // // spin 360deg
  // motor.set_speed(MotorB, Backward, 0);
  // motor.set_speed(MotorA, Forward, 0);

  // // demo obstacle detection
  // // motor move forward
  // motor.set_speed(MotorA, Forward, 255);
  // motor.set_speed(MotorB, Backward, 255);
  // int16_t *value;
  // do
  // {
  //   value = sensors.reading();
  // } while (value[0] >= 0 or value[1] >= 0 or value[2] >= 0); // obstacle not detected

  // // motor move backward for 10 sec
  // motor.set_speed(MotorA, Backward, 96);
  // motor.set_speed(MotorB, Forward, 96);
  // Serial.println("WTF?!\r\n");
  // delay(1000);

  // // motor stop
  // motor.set_speed(MotorA, Forward, 0);
  // motor.set_speed(MotorB, Backward, 0);

  // connect Wi-Fi & AWS
  aws.connectAWS();
}

void loop()
{
  // const int CH_NUM = 3;

  // 0: left, 1: centre, 2: right (on the side with Pololu logo)
  // int16_t *value = sensors.reading();

  // (reading() returns value + 100) mm = distance from the edge of the board
  // I don't know why but the value sometimes become around -1000
  // Serial.printf("\r% 5d % 5d % 5d", value[0], value[1], value[2]);

  aws.publishMessage(8964);
  // aws.publishMessage(value[0], value[1], value[2]);
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

#include <Arduino.h>
#include "AWS.h"
#include "motorDriver.h"
#include "sensorDriver.h"

// enum class led_mode_t
// {
//   FAST,
//   SLOW
// };

// const int LED_BOARD = 2; // change here the pin of the board to V2
// volatile led_mode_t led_mode = led_mode_t::FAST;
// bool led_state = LOW;

// void taskOne(void *parameter);
myawsclass aws = myawsclass();
mclass motor = mclass();
sclass sensors = sclass();

void setup()
{
  // I doubt that GPIO2 has been used from interrupt of sensor input, hence should not be used
  // pinMode(LED_BOARD, OUTPUT);

  // // For deubg LED
  // xTaskCreate(
  //     taskOne,   /* Task function. */
  //     "TaskOne", /* String with name of task. */
  //     /* gonna crash if the stack size is 512 only */
  //     1024,  /* Stack size in bytes. */
  //     NULL,  /* Parameter passed as input of the task */
  //     1,     /* Priority of the task. */
  //     NULL); /* Task handle. */

  Serial.begin(921600);
  sensors.SETUP();

  motor.SETUP();
  motor.set_speed(MotorA, Forward, 0);
  motor.set_speed(MotorB, Backward, 0);

  // connect Wi-Fi & AWS
  aws.connectAWS();
}

void loop()
{
  const int CH_NUM = 3;

  uint16_t distance[CH_NUM]; // 0: left, 1: centre, 2: right (on the side with Pololu logo)
  int16_t *value = sensors.reading();
  for (int i = 0; i < CH_NUM; i++)
  {
    // (reading() returns value + 100) mm = distance from the edge of the board
    distance[i] = 100 + value[i];
  }

  Serial.printf("\r% 4d % 4d % 4d", distance[0], distance[1], distance[2]);
  // Serial.println(*value);

  // Serial.println("Hello from task: 2");
  aws.publishMessage(8964);
  aws.stayConnected();
  delay(1000);
}

// void taskOne(void *parameter)
// {
//   while (true)
//   {
//     led_state = !led_state;
//     // digitalWrite(LED_BOARD, led_state);
//     int delay_ms = (led_mode == led_mode_t::FAST) ? 100 : 500;

//     // delay(1000); //This delay doesn't give a chance to the other tasks to execute
//     vTaskDelay(delay_ms / portTICK_PERIOD_MS); // this pauses the task, so others can execute
//   }
//   vTaskDelete(NULL);
// }

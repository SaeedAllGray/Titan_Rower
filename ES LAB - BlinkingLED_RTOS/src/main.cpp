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

// FreeRTOS
xTimerHandle timerHndl;
void taskLed(void *parameter);
// void taskKinematics(void *parameter);
void vTimerCallbackKinematics(xTimerHandle pxTimer);
void taskAws(void *parameter);

myawsclass aws = myawsclass();
Environment env = Environment();

void setup()
{
  pinMode(LED_BOARD, OUTPUT);

  Serial.begin(921600);

  timerHndl = xTimerCreate(
      "timer1Sec",               /* name */
      pdMS_TO_TICKS(1),          /* period/time */
      pdTRUE,                    /* auto reload */
      (void *)0,                 /* timer ID */
      vTimerCallbackKinematics); /* callback */
  if (timerHndl == NULL)
  {
    Serial.println("Failure! Failure! Failure! Failure!\r\n");
    for (;;)
      ; /* failure! */
  }
  // try to start the timer task until it is possible (e.g. when scheduler not started yet)
  if (xTimerStart(timerHndl, 0) != pdPASS)
  {
    Serial.println("Failure! Failure! Failure! Failure!\r\n");
    for (;;)
      ; /* failure!?! */
  }

  xTaskCreate(
      taskAws,   /* Task function. */
      "taskAws", /* String with name of task. */
      16384,     /* Stack size in bytes. */
      NULL,      /* Parameter passed as input of the task */
      2,         /* Priority of the task. */
      NULL);     /* Task handle. */

  xTaskCreate(
      taskLed,   /* Task function. */
      "taskLed", /* String with name of task. */
      1024,      /* Stack size in bytes. */
      NULL,      /* Parameter passed as input of the task */
      1,         /* Priority of the task. */
      NULL);     /* Task handle. */

  // env.sensors.SETUP();
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
  // sleep(10);
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
  led_mode = led_mode_t::SLOW;
}

void loop()
{
  // const int CH_NUM = 3;

  // // 0: left, 1: centre, 2: right (on the side with Pololu logo)
  // int16_t *value = sensors.reading();

  // // (reading() returns value + 100) mm = distance from the edge of the board
  // // I don't know why but the value sometimes become around -1000
  // Serial.printf("\r% 5d % 5d % 5d", value[0], value[1], value[2]);

  // aws.publishMessage(value[0], value[1], value[2]);
  // delay(1000);
}

void taskLed(void *parameter)
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

void vTimerCallbackKinematics(xTimerHandle pxTimer)
{
  // right: 0, counter-clockwise, 0 - 360
  // [239, 242, 356]
  // [100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119]

  // 10s: speed 255: 95 mm; 127: 43 mm; 63: 17.8 mm
  // linear regression (with Excel): y = 0.40627 x - 7.79732

  // backward^2 10s: 2 7/8 revolutions clockwise
  // wheel base: 115 mm
  // Serial.println(env.currentPos.x);
  // Serial.println(env.currentPos.y);
  // Serial.println(env.currentPos.theta);
  // Serial.println();

  // timer callback every 1 ms

  // <https://blog.classycode.com/esp32-floating-point-performance-6e9f6f567a69>
  // ESP32 can do float sin/cos at ~ 75us, I think we can do <10 times per ms

  // calculate current position
  if (env.currentPosValid){
      xSemaphoreTake(env.currentPosMutex, portMAX_DELAY);

      env.currentPos.x += env.currentVelocity.linear * cos(env.currentPosAngle);
      env.currentPos.y += env.currentVelocity.linear * sin(env.currentPosAngle);
      env.currentPosAngle += env.currentVelocity.angular;

      xSemaphoreGive(env.currentPosMutex);
  }

  // calculate desired velocity
  float tmpVelocity = env.kPLinear * (env.targetPathCurrent->distanceFromPoint(env.currentPos));
  float desiredVelocityLinear = tmpVelocity < env.MAX_LINEAR ? tmpVelocity : env.MAX_LINEAR;

  tmpVelocity = env.kPAngular * (env.targetPathCurrent->angleWithPoint(env.currentPos));
  float desiredVelocityAngular = tmpVelocity < env.MAX_ANGULAR ? tmpVelocity : env.MAX_ANGULAR;

  // calculate motor speed

  // recalculate velocity with motor speed (rounding error)
}

void taskAws(void *parameter)
{

  while (true)
  {
    // aws.publishMessage(8964);
    aws.stayConnected();
    int delay_ms = 10; // 1 ms does work, would get SSL errors
    vTaskDelay(delay_ms / portTICK_PERIOD_MS);
  }
  vTaskDelete(NULL);
}

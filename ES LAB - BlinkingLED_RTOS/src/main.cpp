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

Environment env = Environment();

void setup()
{
  pinMode(LED_BOARD, OUTPUT);

  xTaskCreate(
      taskLed,   /* Task function. */
      "taskLed", /* String with name of task. */
      1024,      /* Stack size in bytes. */
      NULL,      /* Parameter passed as input of the task */
      1,         /* Priority of the task. */
      NULL);     /* Task handle. */

  Serial.begin(921600);

  // env.sensors.SETUP();
  env.motor.SETUP();

  // connect Wi-Fi & AWS
  awsobject.connectAWS();
  led_mode = led_mode_t::SLOW;

  timerHndl = xTimerCreate(
      "timer1Sec",                     /* name */
      pdMS_TO_TICKS(env.TIMER_PERIOD), /* period/time */
      pdTRUE,                          /* auto reload */
      (void *)0,                       /* timer ID */
      vTimerCallbackKinematics);       /* callback */
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
}

void loop()
{
  // const int CH_NUM = 3;

  // // 0: left, 1: centre, 2: right (on the side with Pololu logo)
  // int16_t *value = sensors.reading();

  // // (reading() returns value + 100) mm = distance from the edge of the board
  // // I don't know why but the value sometimes become around -1000
  // Serial.printf("\r% 5d % 5d % 5d", value[0], value[1], value[2]);

  // awsobject.publishMessage(value[0], value[1], value[2]);

  // Serial.println(env.currentPos.x);
  // Serial.println(env.currentPos.y);
  // Serial.println(env.currentPosAngle);
  // Serial.println(env.currentVelocity.linear);
  // Serial.println(env.currentVelocity.angular);
  // Serial.println();
  // delay(1000);
}

void taskLed(void *parameter)
{
  while (true)
  {
    led_state = !led_state;
    digitalWrite(LED_BOARD, led_state);
    int delay_ms = (led_mode == led_mode_t::FAST) ? 100 : 500;

    vTaskDelay(delay_ms / portTICK_PERIOD_MS); // this pauses the task, so others can execute
  }
  vTaskDelete(NULL);
}

void vTimerCallbackKinematics(xTimerHandle pxTimer)
{
  // timer callback every TIMER_PERIOD ms

  // <https://blog.classycode.com/esp32-floating-point-performance-6e9f6f567a69>
  // ESP32 can do float sin/cos at ~ 75us

  // calculate current position
  if (env.currentPosValid)
  {
    xSemaphoreTake(env.currentPosMutex, portMAX_DELAY);

    // TODO: dirty trick, * 0.87 because in reality it move less
    env.currentPos.x += env.currentVelocity.linear * cos(env.currentPosAngle.angle) * env.TIMER_PERIOD / 1000 * 0.87;
    env.currentPos.y += env.currentVelocity.linear * sin(env.currentPosAngle.angle) * env.TIMER_PERIOD / 1000 * 0.87;
    // TODO: I don't know why but it need a /2 here
    // TODO: dirty trick, * 1.1 because in reality it move more
    env.currentPosAngle += env.currentVelocity.angular * env.TIMER_PERIOD / 1000 / 2 * 1.1;

    xSemaphoreGive(env.currentPosMutex);
  }

  float desiredLinear = 0, desiredAngular = 0;
  if (env.debugMode)
  {
    // init by MQTT "tank/debug", override rover velocity
    desiredLinear = env.currentVelocity.linear;
    desiredAngular = env.currentVelocity.angular;
  }
  else
  {
    if (env.currentPosValid && env.targetPathValid)
    {
      // check if current target point reached

      xSemaphoreTake(env.targetPathMutex, portMAX_DELAY);

      Coordinate v = env.targetPath[env.targetPathCurrent];
      // error: diff. btw current position & target point
      float errorLinear = v.distanceFromPoint(env.currentPos);
      float errorAngular = (env.currentPosAngle - v.angleWithPoint(env.currentPos)).angle;
      bool reachedFinalDestination = false;

      while (errorLinear < env.DISTANCE_MARGIN)
      {
        // Serial.printf("target: %d %d;\r\n", env.targetPathCurrent, env.targetPath.size());

        // current target point reached
        if (env.targetPathCurrent >= env.targetPath.size() - 1)
        {
          // already in final destination
          reachedFinalDestination = true;
          Serial.print("DONE ");

          break;
        }
        else
        {
          // Start next point
          env.targetPathCurrent++;
          v = env.targetPath[env.targetPathCurrent];
          errorLinear = v.distanceFromPoint(env.currentPos);
          errorAngular = (env.currentPosAngle - v.angleWithPoint(env.currentPos)).angle;

          Serial.printf("target: %d/%d (%.0f, %.0f)\r\n", env.targetPathCurrent + 1, env.targetPath.size(), v.x, v.y);
        }
      }

      xSemaphoreGive(env.targetPathMutex);

      // calculate desired velocity based on errorLinear & errorAngular

      if (reachedFinalDestination)
      {
        desiredLinear = 0;
        desiredAngular = 0;
      }
      else
      {
        // if abs(errorAngular) > PI / 2 + deadzone (a small number), then flip
        // + deadzone, otherwise it may flip repeatedly
        const float DEADZONE = 10 * 2 * PI / 360; // 5 deg
        if (abs(errorAngular) > PI / 2 + DEADZONE)
        {
          xSemaphoreTake(env.currentPosMutex, portMAX_DELAY);
          env.currentPosAngle += PI;
          errorAngular = (env.currentPosAngle - v.angleWithPoint(env.currentPos)).angle;
          env.roverFlipped = !env.roverFlipped;
          xSemaphoreGive(env.currentPosMutex);
        }

        // Linear
        if (abs(errorAngular) > env.ROTATE_MODE_THOLD)
        {
          // Rotate mode
          desiredLinear = 0;
          Serial.print("R ");
        }
        else
        {
          // Forward mode
          float tmpLinear = env.kP_LINEAR * (errorLinear);
          desiredLinear = constrain(tmpLinear, -env.MAX_LINEAR, env.MAX_LINEAR);
          Serial.print("F ");
        }

        // Angular
        float tmpAngular = abs(errorAngular) < env.ANGLE_MARGIN ? 0 : env.kP_ANGULAR * (-errorAngular);
        desiredAngular = constrain(tmpAngular, -env.MAX_ANGULAR, env.MAX_ANGULAR);
        // Serial.printf("%.2f %.2f %.2f\r\n", errorAngular, v.angleWithPoint(env.currentPos).angle, desiredAngular);
      }
    }
  }

  // calculate motor speed

  // speed diff. from left motor to right motor, in order to meet desiredAngular
  // in small angle, chord length == arc length == angle (in radian) * radius (rover width / 2)
  float speedDiff = desiredAngular * env.ROVER_WID;
  float desiredSpeed[2];                           //, actualSpeed[2];
  desiredSpeed[0] = desiredLinear + speedDiff / 2; // left
  desiredSpeed[1] = desiredLinear - speedDiff / 2; // right

  // // if roverFlipped, the rover should move in the opposite direction
  // for (int i = 0; i < 2; i++)
  // {
  //   if (env.roverFlipped)
  //   {
  //     desiredSpeed[i] = -desiredSpeed[i];
  //   }
  // }

  // cap speed if exceed maximum
  for (int i = 0; i < 2; i++)
    if (desiredSpeed[i] > env.MAX_LINEAR)
    {
      desiredSpeed[i] = env.MAX_LINEAR;
      desiredSpeed[i == 0 ? 1 : 0] = env.MAX_LINEAR - speedDiff;
      desiredLinear = (desiredSpeed[0] + desiredSpeed[1]) / 2;
      break; // only 1 of the 2 wheel may exist the max speed
    }

  env.setMotorSpeed(desiredSpeed); //, actualSpeed);

  // // recalculate velocity with motor speed (rounding error)
  // env.currentVelocity.linear = (actualSpeed[0] + actualSpeed[1]) / 2;
  // env.currentVelocity.angular = (actualSpeed[1] - actualSpeed[0]) / env.ROVER_WID;
  env.currentVelocity.linear = desiredLinear;
  env.currentVelocity.angular = desiredAngular;

  Serial.printf("pos: (%.2f, %.2f, %.4f);\t", env.currentPos.x, env.currentPos.y, env.currentPosAngle.angle);
  Serial.printf("Motor speed: (%.2f, %.2f) (%d, %d);\t", desiredSpeed[0], desiredSpeed[1], env.currentMotorSpeed[0], env.currentMotorSpeed[1]);
  Serial.printf("speed: (%.2f, %.2f)\r\n", env.currentVelocity.linear, env.currentVelocity.angular);
}

void taskAws(void *parameter)
{

  while (true)
  {
    // awsobject.publishMessage(8964);
    awsobject.stayConnected();
    int delay_ms = 10; // 1 ms does work, would get SSL errors
    vTaskDelay(delay_ms / portTICK_PERIOD_MS);
  }
  vTaskDelete(NULL);
}

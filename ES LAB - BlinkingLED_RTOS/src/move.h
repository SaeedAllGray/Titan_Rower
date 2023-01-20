// #include <Arduino.h>
// // #include "AWS.h"
// #include "motorDriver.h"
// #include "sensorDriver.h"
// #include "coordinate.h"
// void goStraight(mclass& motor, int speed)
// {
//     do
//     {
//         motor.set_speed(MotorB, Forward, speed);
//         motor.set_speed(MotorA, Backward, speed);
//         Serial.println(currentPosition.x);

//         delay(300);
//     } while (currentPosition.x != targetPosition.x);
//     motor.set_speed(MotorB, Forward, 0);
//     motor.set_speed(MotorA, Backward, 0);
// }

// void turnLeft(int speed)
// {
//     do
//     {
//         motor.set_speed(MotorA, Backward, speed);
//         motor.set_speed(MotorB, Backward, speed);
//         Serial.println(currentPosition.x);

//         delay(300);
//     } while (currentPosition.angleWithTarget(targetPosition) != 0);
//     motor.set_speed(MotorB, Forward, 0);
//     motor.set_speed(MotorA, Backward, 0);
// }

// void turnRight(int speed)
// {
//     do
//     {
//         motor.set_speed(MotorA, Forward, speed);
//         motor.set_speed(MotorB, Forward, speed);
//         Serial.println(currentPosition.x);

//         delay(300);
//     } while (currentPosition.angleWithTarget(targetPosition) != 0);
//     motor.set_speed(MotorB, Forward, 0);
//     motor.set_speed(MotorA, Backward, 0);
// }
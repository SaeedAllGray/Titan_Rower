#include <Arduino.h>
// #include "AWS.h"
#include "motorDriver.h"
#include "sensorDriver.h"
#include "coordinate.h"

class Environment
{
public:
    mclass motor = mclass();
    sclass sensors = sclass();
    Coordinate currentPosition = Coordinate(0, 0);
    Coordinate targetPosition = Coordinate(10, 20);

    void goForward(int speed)
    {
        do
        {
            motor.set_speed(MotorB, Forward, speed);
            motor.set_speed(MotorA, Backward, speed);
            Serial.println(currentPosition.x);

            delay(300);
        } while (currentPosition.x != targetPosition.x);
        motor.set_speed(MotorB, Forward, 0);
        motor.set_speed(MotorA, Backward, 0);
    }
    void goBackward(int speed)
    {
        do
        {
            motor.set_speed(MotorB, Backward, speed);
            motor.set_speed(MotorA, Forward, speed);
            currentPosition.x += 1;
            Serial.println(currentPosition.x);

            delay(300);
        } while (currentPosition.x != targetPosition.x);
        motor.set_speed(MotorB, Forward, 0);
        motor.set_speed(MotorA, Backward, 0);
    }

    void turnLeft(int speed)
    {
        do
        {
            motor.set_speed(MotorA, Backward, speed);
            motor.set_speed(MotorB, Backward, speed);
            Serial.println(currentPosition.x);

            delay(300);
        } while (currentPosition.angleWithTarget(targetPosition) != 0);
        motor.set_speed(MotorB, Forward, 0);
        motor.set_speed(MotorA, Backward, 0);
    }

    void turnRight(int speed)
    {
        do
        {
            motor.set_speed(MotorA, Forward, speed);
            motor.set_speed(MotorB, Forward, speed);
            Serial.println(currentPosition.x);

            delay(300);
        } while (currentPosition.angleWithTarget(targetPosition) != 0);
        motor.set_speed(MotorB, Forward, 0);
        motor.set_speed(MotorA, Backward, 0);
    }
};
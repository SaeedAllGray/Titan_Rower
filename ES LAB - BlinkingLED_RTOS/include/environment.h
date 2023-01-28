#ifndef environment_h
#define environment_h

#include <Arduino.h>
#include <vector>
// #include "AWS.h"
#include "motorDriver.h"
#include "sensorDriver.h"
#include "coordinate.h"

const int MAX_TARGET_PTS = 10;

const int DISTANCE_MARGIN = 5;
const float ANGLE_MARGIN = 5 * PI / 360; // in radian

class Environment
{
public:

    struct Velocity
    {
        float linear;
        float angular;
    };

    mclass motor = mclass();
    sclass sensors = sclass();

    Coordinate currentPos = Coordinate(-1, -1);
    float currentPosAngle = 0.0; // in radian
    bool currentPosValid = false;
    SemaphoreHandle_t currentPosMutex = xSemaphoreCreateMutex();
    
    Coordinate targetPosition = Coordinate(-1, -1);

    std::vector<Coordinate> targetPath;
    bool targetPathValid = false;
	std::vector<Coordinate>::iterator targetPathCurrent;

    const int kPLinear = 100;
    const int kPAngular = 1;
    const float MAX_LINEAR = 9.5 / 1000; // 9.5 mm / s
    const float MAX_ANGULAR = 0.2875 * 2 * PI / 1000; // 0.2875 revolution / s, unit: rad / ms
    // for kinematics
    // [0]: linear, mm / ms; [1]: angular, rad / ms
    Velocity currentVelocity = {.linear = 0.0, .angular = 0.0};
    int currentMotorSpeed[2] = {};

    // for motor control

    void goForward(int speed)
    {
        do
        {
            motor.set_speed(MotorB, Forward, speed);
            motor.set_speed(MotorA, Backward, speed);
            Serial.println(currentPos.x);

            delay(300);
        } while (abs(currentPos.distanceFromPoint(targetPosition)) < DISTANCE_MARGIN);
        motor.set_speed(MotorB, Forward, 0);
        motor.set_speed(MotorA, Backward, 0);
    }
    void goBackward(int speed)
    {
        do
        {
            motor.set_speed(MotorB, Backward, speed);
            motor.set_speed(MotorA, Forward, speed);
            Serial.println(currentPos.x);

            delay(300);
        } while (abs(currentPos.distanceFromPoint(targetPosition)) < DISTANCE_MARGIN);
        motor.set_speed(MotorB, Forward, 0);
        motor.set_speed(MotorA, Backward, 0);
    }

    void turnLeft(int speed)
    {
        do
        {
            motor.set_speed(MotorA, Backward, speed);
            motor.set_speed(MotorB, Backward, speed);
            Serial.println(currentPos.x);

            delay(300);
        } while (abs(currentPos.angleWithPoint(targetPosition) - currentPosAngle) < ANGLE_MARGIN);
        motor.set_speed(MotorB, Forward, 0);
        motor.set_speed(MotorA, Backward, 0);
    }

    void turnRight(int speed)
    {
        do
        {
            motor.set_speed(MotorA, Forward, speed);
            motor.set_speed(MotorB, Forward, speed);
            Serial.println(currentPos.x);

            delay(300);
        } while (abs(currentPos.angleWithPoint(targetPosition) - currentPos.theta) < ANGLE_MARGIN);
        motor.set_speed(MotorB, Forward, 0);
        motor.set_speed(MotorA, Backward, 0);
    }
};

#endif

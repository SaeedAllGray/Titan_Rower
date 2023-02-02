#ifndef environment_h
#define environment_h

#include <Arduino.h>
#include <vector>
// #include "AWS.h"
#include "motorDriver.h"
#include "sensorDriver.h"
#include "coordinate.h"
#include "NormalizedAngle.h"

const int MAX_TARGET_PTS = 10;

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

    // right: 0, counter-clockwise, 0 - 360
    // [239, 242, 356]
    // [100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119]

    const int TIMER_PERIOD = 10; // period of vTimerCallbackKinematics(), in ms

    // to calculate rover rotation
    // measured: 115 (distance btw wheels) mm
    // calculated from linear speed -> 105.1 mm
    const float ROVER_WID = 105;

    // max. speeds
    const float MAX_LINEAR = 95; // 95 mm / s

    // // experiment: backward^2 10s: 2 7/8 revolutions clockwise
    // const float MAX_ANGULAR = 0.2875 * 2 * PI; // 0.2875 revolution / s, unit: rad / s

    // angular speed = linear speed * radius (rover width)
    // doubled beacuse both wheels can move in opposite directions, doubling the angular speed
    const float MAX_ANGULAR = MAX_LINEAR / ROVER_WID * 2;

    // P controller for rover velocity
    const float kP_LINEAR = 1;
    const float kP_ANGULAR = 7;

    // margin of target points
    // pixels_target_tolerance in configureSystem.py is 20 / 480 * 1300 = 54.2 mm
    const float DISTANCE_MARGIN = 40;
    const float ANGLE_MARGIN = 5 * PI / 360; // in radian

    const float ROTATE_MODE_THOLD = 30 * PI / 360; // in radian

    // coordinates
    Coordinate currentPos = Coordinate(0, 0);
    NormalizedAngle currentPosAngle = NormalizedAngle(0.0); // in radian
    bool currentPosValid = false;
    bool roverFlipped = false;
    SemaphoreHandle_t currentPosMutex = xSemaphoreCreateMutex();

    // legacy
    Coordinate targetPosition = Coordinate(0, 0);

    // target path
    std::vector<Coordinate> targetPath;
    bool targetPathValid = false;
    int targetPathCurrent = 0;
    SemaphoreHandle_t targetPathMutex = xSemaphoreCreateMutex();

    // init by MQTT "tank/debug", override rover velocity
    bool debugMode = false;

    // for kinematics
    // [0]: linear, mm / s; [1]: angular, rad / s
    Velocity currentVelocity = {.linear = 0.0, .angular = 0.0};
    int currentMotorSpeed[2] = {};

    /// @brief
    /// @param desired input, desired motor speed, [0]: left, [1]: right
    ///// @param actual  output, actual motor speed because of rounding error
    void setMotorSpeed(float desired[2]) //, float actual[2])
    {
        // experiment: 10s: speed 255: 950 mm;
        //                        127: 430 mm;
        //                         63: 178 mm
        // linear regression (with Excel): actual_speed = 0.406(speed_input) - 7.797
        //   x-intercept = => set to 0 if smaller than x-intercept
        const float M = 0.406278571428672;
        const float C = -7.79732142857144;
        const int X_INTERCEPT = static_cast<int>(-C / M);

        float speed[2];

        // [0]: Left; [1]: Right
        for (int i = 0; i < 2; i++)
        {
            // if roverFlipped, the left motor becomes the right motor, and vice versa
            int motorSpeed = static_cast<int>((abs(desired[roverFlipped ? 1 - i : i]) - C) / M);

            // constrain motor speed
            motorSpeed = constrain(motorSpeed, 0, 255);
            if (motorSpeed <= X_INTERCEPT)
            {
                motorSpeed = 0;
            }

            // moveMotorForward: forward in repsect to rover direction, ignoring roverFlipped
            // if roverFlipped, flip motor direction -> use XOR (^)
            bool moveMotorForward = (desired[roverFlipped ? 1 - i : i] > 0) ^ roverFlipped;
            if (i == 0)
            {
                Direction dir = moveMotorForward ? Forward : Backward;
                motor.set_speed(MotorA, dir, motorSpeed);
            }
            else
            {
                Direction dir = moveMotorForward ? Backward : Forward;
                motor.set_speed(MotorB, dir, motorSpeed);
            }

            // for debug messages
            currentMotorSpeed[i] = moveMotorForward ? motorSpeed : -motorSpeed;
            // Serial.printf("Motor %d: %d %d\r\n", i, motorSpeed, currentMotorSpeed[i]);

            // // recalculate actual speed
            // actual[i] = currentMotorSpeed[i] * M + C;
            // actual[i] = actual[i] < 0 ? 0 : actual[i];
        }
    }

    void goForward(int speed)
    {
        do
        {
            motor.set_speed(MotorA, Forward, speed);
            motor.set_speed(MotorB, Backward, speed);
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
            motor.set_speed(MotorA, Backward, speed);
            motor.set_speed(MotorB, Forward, speed);
            Serial.println(currentPos.x);

            delay(300);
        } while (abs(currentPos.distanceFromPoint(targetPosition)) < DISTANCE_MARGIN);
        motor.set_speed(MotorB, Forward, 0);
        motor.set_speed(MotorA, Backward, 0);
    }

    void turnRight(int speed)
    {
        do
        {
            motor.set_speed(MotorA, Backward, speed);
            motor.set_speed(MotorB, Backward, speed);
            Serial.println(currentPos.x);

            delay(300);
        } while (abs((currentPos.angleWithPoint(targetPosition) - currentPosAngle).angle) < ANGLE_MARGIN);
        motor.set_speed(MotorB, Forward, 0);
        motor.set_speed(MotorA, Backward, 0);
    }

    void turnLeft(int speed)
    {
        do
        {
            motor.set_speed(MotorA, Forward, speed);
            motor.set_speed(MotorB, Forward, speed);
            Serial.println(currentPos.x);

            delay(300);
        } while (abs((currentPos.angleWithPoint(targetPosition) - currentPosAngle).angle) < ANGLE_MARGIN);
        motor.set_speed(MotorB, Forward, 0);
        motor.set_speed(MotorA, Backward, 0);
    }
};

#endif

#ifndef NORMALIZED_ANGLE_H
#define NORMALIZED_ANGLE_H

#include <cmath>
#include <Arduino.h> // PI

class NormalizedAngle
{
public:
    float angle; // range [-pi, pi]

    NormalizedAngle(float angle = 0.0) : angle(angle) {}
    void operator=(float b);
    void operator+=(float b);
    void operator-=(float b);

private:
    void normalize();
    friend NormalizedAngle operator+(NormalizedAngle a, NormalizedAngle b);
    friend NormalizedAngle operator-(NormalizedAngle a, NormalizedAngle b);
    friend NormalizedAngle operator+(NormalizedAngle a, float b);
    friend NormalizedAngle operator-(NormalizedAngle a, float b);
};

#endif


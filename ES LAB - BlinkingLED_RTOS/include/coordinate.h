#ifndef coordinate_h
#define coordinate_h

#include <math.h>
#include <bits/stdc++.h>
class Coordinate
{
public:
    float x;
    float y;

    Coordinate() : x(-1), y(-1) {}
    Coordinate(float xin, float yin) : x(xin), y(yin) {}

    float angleWithPoint(Coordinate point)
    {
        float deltaY = (*this).y - point.y;
        float deltaX = (*this).x - point.x;
        return atan2(deltaY, deltaX);
    }
    float distanceFromPoint(Coordinate point)
    {
        float x2 = pow((*this).x - point.x, 2);
        float y2 = pow((*this).y - point.y, 2);

        return sqrt(x2 + y2);
    }
};

#endif

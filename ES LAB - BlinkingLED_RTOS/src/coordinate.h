#include <math.h>
#include <bits/stdc++.h>
class Coordinate
{
public:
    float x;
    float y;

    Coordinate(int xin, int yin) : x(xin), y(yin) {}

    int angleWithTarget(Coordinate point1)
    {
        float deltaY = (*this).y - point1.y;
        float deltaX = (*this).x - point1.x;
        return int(atan(deltaY / deltaX));
    }
};

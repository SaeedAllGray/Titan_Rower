#include <math.h>
#include <bits/stdc++.h>
class Coordinate
{
public:
    float x;
    float y;

    Coordinate(int xin, int yin) : x(xin), y(yin) {}

    int angleWithPoint(Coordinate point)
    {
        float deltaY = (*this).y - point.y;
        float deltaX = (*this).x - point.x;
        return int(atan(deltaY / deltaX));
    }
    double distanceFromPoint(Coordinate point)
    {
        double x2 = pow((*this).x - point.x,2);
        double y2 = pow((*this).y - point.y, 2);

        return sqrt(x2 + y2);
    }
};

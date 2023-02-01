#include "NormalizedAngle.h"

void NormalizedAngle::normalize()
{
    angle = remainder(angle, PI * 2);
}

void NormalizedAngle::operator=(float b)
{
    angle = b;
    normalize();
}

void NormalizedAngle::operator+=(float b)
{
    angle += b;
    normalize();
}

void NormalizedAngle::operator-=(float b)
{
    angle -= b;
    normalize();
}

NormalizedAngle operator+(NormalizedAngle a, NormalizedAngle b)
{
    NormalizedAngle na = NormalizedAngle(a.angle + b.angle);
    na.normalize();
    return na;
}

NormalizedAngle operator-(NormalizedAngle a, NormalizedAngle b)
{
    NormalizedAngle nb = NormalizedAngle(-(b.angle));
    return a + nb;
}

NormalizedAngle operator+(NormalizedAngle a, float b)
{
    NormalizedAngle na = NormalizedAngle(a.angle + b);
    na.normalize();
    return na;
}

NormalizedAngle operator-(NormalizedAngle a, float b)
{
    return a + (-b);
}

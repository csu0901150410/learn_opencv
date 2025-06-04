#include "lsPoint.h"

lsPoint::lsPoint()
	: x(0), y(0)
{
}

lsPoint::lsPoint(lsReal vx, lsReal vy)
	: x(vx), y(vy)
{
}

lsPoint::lsPoint(const lsPoint& other)
	: x(other.x), y(other.y)
{
}

lsPoint lsPoint::operator-(const lsPoint& other) const
{
	return lsPoint(x - other.x, y - other.y);
}

lsPoint lsPoint::operator+=(const lsPoint& other)
{
	x += other.x;
	y += other.y;
	return *this;
}

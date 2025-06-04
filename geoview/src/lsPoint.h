#pragma once

#include "lsDefs.h"

class lsPoint
{
public:
	lsPoint();
	lsPoint(lsReal vx, lsReal vy);
	lsPoint(const lsPoint& other);
	~lsPoint() = default;

	lsPoint operator-(const lsPoint& other) const;
	lsPoint operator+=(const lsPoint& other);

public:
	lsReal x;
	lsReal y;
};


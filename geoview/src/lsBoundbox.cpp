#include "lsBoundbox.h"

#include <algorithm>

lsBoundbox::lsBoundbox()
{
	init();
}

lsBoundbox::lsBoundbox(const lsBoundbox& box)
	: left(box.left)
	, right(box.right)
	, bottom(box.bottom)
	, top(box.top)
{
}

lsBoundbox::lsBoundbox(lsReal x, lsReal y, lsReal w, lsReal h)
{
	left = x;
	bottom = y;
	right = x + w;
	top = y + h;
}

void lsBoundbox::init()
{
	left = LSMAX;
	right = LSMIN;
	bottom = LSMAX;
	top = LSMIN;
}

bool lsBoundbox::is_valid() const
{
	if (left >= right || bottom >= top)
		return false;
	return true;
}

lsReal lsBoundbox::width() const
{
	return right - left;
}

lsReal lsBoundbox::height() const
{
	return top - bottom;
}

lsReal lsBoundbox::area() const
{
	return width() * height();
}

lsPoint lsBoundbox::center() const
{
	return lsPoint((left + right) / 2, (bottom + top) / 2);
}

bool lsBoundbox::is_in(const lsPoint& point)
{
	if (point.x < left || point.x > right)
		return false;
	if (point.y < bottom || point.y > top)
		return false;
	return true;
}

bool lsBoundbox::is_contain(const lsBoundbox& box)
{
	lsPoint c1(left, bottom), c2(right, top);
	return is_in(c1) && is_in(c2);
}

bool lsBoundbox::is_intersect(const lsBoundbox& box) const
{
	if (left > box.right || right < box.left
		|| bottom > box.top || top < box.bottom)
		return false;
	return true;
}

void lsBoundbox::offset(lsReal dx, lsReal dy)
{
	left -= dx;
	right += dx;
	bottom -= dy;
	top += dy;
}

void lsBoundbox::scale(lsReal sx, lsReal sy)
{
	if (sx <= 0 || sy <= 0)
		return;

	lsReal dx = (right - left) * (sx - 1) / 2;
	lsReal dy = (top - bottom) * (sy - 1) / 2;
	offset(dx, dy);
}

void lsBoundbox::intersect(const lsBoundbox& box)
{
	if (!is_intersect(box))
	{
		init();
	}
	else
	{
		left = std::max(left, box.left);
		right = std::min(right, box.right);
		bottom = std::max(bottom, box.bottom);
		top = std::min(top, box.top);
	}
}

void lsBoundbox::combine(const lsBoundbox& box)
{
	left = std::min(left, box.left);
	right = std::max(right, box.right);
	bottom = std::min(bottom, box.bottom);
	top = std::max(top, box.top);
}

lsBoundbox lsBoundbox::intersect_with(const lsBoundbox& box) const
{
	lsBoundbox ret = *this;
	ret.intersect(box);
	return ret;
}

lsBoundbox lsBoundbox::combine_with(const lsBoundbox& box) const
{
	lsBoundbox ret = *this;
	ret.combine(box);
	return ret;
}

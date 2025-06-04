#pragma once

#include <wx/geometry.h>

#include "lsEntity.h"

#include "lsPoint.h"

class lsBoundbox;

class lsLine : public lsEntity
{
public:
	lsLine(const lsPoint& s, const lsPoint& e, double width);

	void Draw(lsRenderer& renderer) const override;

	lsBoundbox GetBoundBox() const override;

	bool HitTest(const lsPoint& pos, double tol) const override;

	bool IntersectWith(const lsBoundbox& box) const override;

private:
	lsPoint s;
	lsPoint e;
	double r;
};

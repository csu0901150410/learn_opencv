#pragma once

#include <wx/geometry.h>

#include "lsEntity.h"

class lsLine : public lsEntity
{
public:
	lsLine(const wxPoint2DDouble& s, const wxPoint2DDouble& e, double width);

	void Draw(lsRenderer& renderer) const override;

	wxRect2DDouble GetBoundBox() const override;

private:
	wxPoint2DDouble s;
	wxPoint2DDouble e;
	double r;
};

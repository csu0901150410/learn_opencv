#include "lsLine.h"

#include "lsCanvas.h"

#include "lsBoundbox.h"

lsLine::lsLine(const lsPoint& s_, const lsPoint& e_, double width_)
	: s(s_)
	, e(e_)
	, r(width_)
{

}

void lsLine::Draw(lsRenderer& renderer) const
{
	if (m_selected)
		renderer.SetHighlight(true);
	else
		renderer.SetHighlight(false);

	renderer.SetLineWidth(r);
	renderer.DrawLine(s, e);
}

lsBoundbox lsLine::GetBoundBox() const
{
	double minx = std::min(s.x, e.x);
	double miny = std::min(s.y, e.y);
	double maxx = std::max(s.x, e.x);
	double maxy = std::max(s.y, e.y);
	return lsBoundbox(minx, miny, maxx - minx, maxy - miny);
}

bool lsLine::HitTest(const lsPoint& p, double tol) const
{
	// point to segment distance
	lsPoint se(e - s);
	lsPoint sp(p - s);

	double se2 = se.x * se.x + se.y * se.y;
	double t = (sp.x * se.x + sp.y * se.y) / se2;

	lsPoint proj;
	if (t <= 0.0)
	{
		proj = s;
	}
	else if (t >= 1.0)
	{
		proj = e;
	}
	else
	{
		proj = lsPoint(s.x + t * se.x, s.y + t * se.y);
	}

	double dx = p.x - proj.x;
	double dy = p.y - proj.y;
	double dis = std::sqrt(dx * dx + dy * dy);

	return (dis <= tol);
}

bool lsLine::IntersectWith(const lsBoundbox& box) const
{
	// 要考虑线段是否和box真正相交
	lsBoundbox segbox = GetBoundBox();
	return box.is_intersect(segbox);
}


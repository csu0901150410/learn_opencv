#include "lsLine.h"

#include "lsCanvas.h"

lsLine::lsLine(const wxPoint2DDouble& s_, const wxPoint2DDouble& e_, double width_)
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

wxRect2DDouble lsLine::GetBoundBox() const
{
	double minx = std::min(s.m_x, e.m_x);
	double miny = std::min(s.m_y, e.m_y);
	double maxx = std::max(s.m_x, e.m_x);
	double maxy = std::max(s.m_y, e.m_y);
	return wxRect2DDouble(minx, miny, maxx - minx, maxy - miny);
}

bool lsLine::HitTest(const wxPoint2DDouble& p, double tol) const
{
	// point to segment distance
	wxPoint2DDouble se(e - s);
	wxPoint2DDouble sp(p - s);

	double se2 = se.m_x * se.m_x + se.m_y * se.m_y;
	double t = (sp.m_x * se.m_x + sp.m_y * se.m_y) / se2;

	wxPoint2DDouble proj;
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
		proj = wxPoint2DDouble(s.m_x + t * se.m_x, s.m_y + t * se.m_y);
	}

	double dx = p.m_x - proj.m_x;
	double dy = p.m_y - proj.m_y;
	double dis = std::sqrt(dx * dx + dy * dy);

	return (dis <= tol);
}

bool lsLine::IntersectWith(const wxRect2DDouble& box) const
{
	// 要考虑线段是否和box真正相交
	wxRect2DDouble segbox = GetBoundBox();
	return box.Intersects(segbox);
}


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


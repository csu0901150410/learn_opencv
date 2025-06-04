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
	// Cohen-Sutherland 线段裁剪算法
	// See https://en.wikipedia.org/wiki/Cohen%E2%80%93Sutherland_algorithm
	// See https://sighack.com/post/cohen-sutherland-line-clipping-algorithm

	// 策略1
	// 如果线段两个端点都在box的一侧，则线段完全在box外，线段和box不相交
	if (s.x < box.left && e.x < box.left)
		return false;
	if (s.x > box.right && e.x > box.right)
		return false;
	if (s.y < box.bottom && e.y < box.bottom)
		return false;
	if (s.y > box.top && e.y > box.top)
		return false;

	// 策略二
	// 如果线段完全在box内，则线段和box相交，这种比较适合框选逻辑
	if (box.is_in(s) && box.is_in(e))
		return true;
	else
		return false;

	return true;
}

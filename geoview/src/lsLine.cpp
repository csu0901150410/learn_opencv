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

	auto encode = [](lsReal x, lsReal y, lsReal clipx, lsReal clipy, lsReal clipw, lsReal cliph) {
		int code = 0;
		
		lsReal xmin = clipx;
		lsReal ymin = clipy;
		lsReal xmax = clipx + clipw;
		lsReal ymax = clipy + cliph;

		// 点在裁剪区域左侧，置位bit-0
		if (x < xmin)
			code |= (1 << 0);
		// 点在裁剪区域右侧，置位bit-1
		else if (x > xmax)
			code |= (1 << 1);
		
		// 点在裁剪区域下方，置位bit-2
		if (y < ymin)
			code |= (1 << 2);
		// 点在裁剪区域上方，置位bit-3
		else if (y > ymax)
			code |= (1 << 3);

		return code;
	};

	lsReal clipx = box.left;
	lsReal clipy = box.bottom;
	lsReal clipw = box.width();
	lsReal cliph = box.height();

	int code0 = 0, code1 = 0;
	lsReal xmin = clipx;
	lsReal ymin = clipy;
	lsReal xmax = clipx + clipw;
	lsReal ymax = clipy + cliph;

	// true-线段和裁剪区域相交
	bool bAccept = false;
	lsReal x0 = s.x;
	lsReal y0 = s.y;
	lsReal x1 = e.x;
	lsReal y1 = e.y;

	do
	{
		code0 = encode(x0, y0, clipx, clipy, clipw, cliph);
		code1 = encode(x1, y1, clipx, clipy, clipw, cliph);

		// 线段两个端点都在裁剪区域内，则线段在裁剪区域内
		if (0 == code0 && 0 == code1)
		{
			bAccept = true;
			break;
		}

		// 端点编码位与不为0，则编码在相同的位上同时为1
		// 表明两个端点位于裁剪区域同一侧，线段完全在裁剪区域外
		else if (0 != (code0 & code1))
		{
			break;
		}

		// 选一个裁剪区域外的端点进行裁剪，裁剪到裁剪区域的边界
		// 分两种情况
		// 1、两个端点都在区域外，线段穿过区域或者不穿过区域
		// 2、两个端点分布在区域两侧
		// 注意，对于端点在区域外不同侧并且线段未穿过区域的，要计算裁剪，这是低效的
		else
		{
			double x = 0, y = 0;
			int code = 0 != code0 ? code0 : code1;

			// 注意以下各情况(x1 - x0)和(y1 - y0)

			// 端点在裁剪区域左侧
			if (0 != (code & (1 << 0)))
			{
				x = xmin;
				y = (y1 - y0) * (x - x0) / (x1 - x0) + y0;
			}
			// 端点在裁剪区域右侧
			else if (0 != (code & (1 << 1)))
			{
				x = xmax;
				y = (y1 - y0) * (x - x0) / (x1 - x0) + y0;
			}
			// 端点在裁剪区域下方
			else if (0 != (code & (1 << 2)))
			{
				y = ymin;
				x = (x1 - x0) * (y - y0) / (y1 - y0) + x0;
			}
			// 端点在裁剪区域上方
			else if (0 != (code & (1 << 3)))
			{
				y = ymax;
				x = (x1 - x0) * (y - y0) / (y1 - y0) + x0;
			}
			// 端点在裁剪区域内部
			else
			{
				// 此时端点不可能在区域内部
			}

			// 更新被裁剪的端点
			if (code == code0)
			{
				x0 = x;
				y0 = y;
			}
			else
			{
				x1 = x;
				y1 = y;
			}
		}
	} while (true);

	return bAccept;
}

#include "lsViewControl.h"
#include "lsBoundbox.h"

lsPoint lsViewControl::WorldToScreen(const lsPoint& pt) const
{
	/*lsPoint screen;
	screen.m_x = pt.m_x * m_scale + m_offset.m_x;
	screen.m_y = pt.m_y * m_scale + m_offset.m_y;
	return screen;*/

	wxPoint2DDouble pos(pt.x, pt.y);
	pos = GetWorldToScreenMatrix().TransformPoint(pos);

	return lsPoint(pos.m_x, pos.m_y);
}

lsPoint lsViewControl::ScreenToWorld(const lsPoint& pt) const
{
	/*lsPoint world;
	world.m_x = (pt.m_x - m_offset.m_x) / m_scale;
	world.m_y = (pt.m_y - m_offset.m_y) / m_scale;
	return world;*/

	wxPoint2DDouble pos(pt.x, pt.y);
	pos = GetScreenToWorldMatrix().TransformPoint(pos);

	return lsPoint(pos.m_x, pos.m_y);
}

void lsViewControl::Zoom(double factor, const lsPoint& anchor_screen)
{
	// 在屏幕坐标 anchor_screen 处进行缩放，缩放前后，屏幕点和世界点保持对应，更新平移和缩放量
	lsPoint anchor_world = ScreenToWorld(anchor_screen);

	m_scale *= factor;

	// 更新所放量之后，世界点对到其他的屏幕点上了，需要用屏幕坐标系下的平移量移回原来的屏幕点
	lsPoint new_anchor_screen = WorldToScreen(anchor_world);

	// 新屏幕点指向旧屏幕点的平移向量，是应用新缩放量之后所需的额外的平移量，所以要加在旧平移量上
	m_offset += (anchor_screen - new_anchor_screen);
}

void lsViewControl::Pan(double dx, double dy)
{
	m_offset.x += dx;
	m_offset.y += dy;
}

void lsViewControl::ZoomToFit(const lsBoundbox& worldBox, const wxSize& screenSize)
{
	if (worldBox.width() <= 0 || worldBox.height() <= 0 
		|| screenSize.GetWidth() <= 0 || screenSize.GetHeight() <= 0)
		return;

	double paddingFactor = 0.9;

	double scalex = screenSize.GetWidth() / worldBox.width();
	double scaley = screenSize.GetHeight() / worldBox.height();
	m_scale = std::min(scalex, scaley) * paddingFactor;

	double screenw = worldBox.width() * m_scale;
	double screenh = worldBox.height() * m_scale;

	double extrax = (screenSize.GetWidth() - screenw) / 2.0;
	double extray = (screenSize.GetHeight() - screenh) / 2.0;

	m_offset = lsPoint(-worldBox.left + extrax / m_scale,
		-worldBox.bottom + extray / m_scale);
}

wxAffineMatrix2D lsViewControl::GetWorldToScreenMatrix() const
{
	wxAffineMatrix2D mat;
	mat.Translate(m_offset.x, m_offset.y);
	mat.Scale(m_scale, m_scale);
	return mat;
}

wxAffineMatrix2D lsViewControl::GetScreenToWorldMatrix() const
{
	wxAffineMatrix2D inv = GetWorldToScreenMatrix();
	inv.Invert();
	return inv;
}

double lsViewControl::GetZoomFactor() const
{
	return m_scale;
}


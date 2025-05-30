#include "lsViewControl.h"

wxPoint2DDouble lsViewControl::WorldToScreen(const wxPoint2DDouble& pt) const
{
	/*wxPoint2DDouble screen;
	screen.m_x = pt.m_x * m_scale + m_offset.m_x;
	screen.m_y = pt.m_y * m_scale + m_offset.m_y;
	return screen;*/

	return GetWorldToScreenMatrix().TransformPoint(pt);
}

wxPoint2DDouble lsViewControl::ScreenToWorld(const wxPoint2DDouble& pt) const
{
	/*wxPoint2DDouble world;
	world.m_x = (pt.m_x - m_offset.m_x) / m_scale;
	world.m_y = (pt.m_y - m_offset.m_y) / m_scale;
	return world;*/

	return GetScreenToWorldMatrix().TransformPoint(pt);
}

void lsViewControl::Zoom(double factor, const wxPoint2DDouble& anchor_screen)
{
	// 在屏幕坐标 anchor_screen 处进行缩放，缩放前后，屏幕点和世界点保持对应，更新平移和缩放量
	wxPoint2DDouble anchor_world = ScreenToWorld(anchor_screen);

	m_scale *= factor;

	// 更新所放量之后，世界点对到其他的屏幕点上了，需要用屏幕坐标系下的平移量移回原来的屏幕点
	wxPoint2DDouble new_anchor_screen = WorldToScreen(anchor_world);

	// 新屏幕点指向旧屏幕点的平移向量，是应用新缩放量之后所需的额外的平移量，所以要加在旧平移量上
	m_offset += (anchor_screen - new_anchor_screen);
}

void lsViewControl::Pan(double dx, double dy)
{
	m_offset.m_x += dx;
	m_offset.m_y += dy;
}

void lsViewControl::ZoomToFit(const wxRect2DDouble& worldBox, const wxSize& screenSize)
{
	if (worldBox.m_width <= 0 || worldBox.m_height <= 0 
		|| screenSize.GetWidth() <= 0 || screenSize.GetHeight() <= 0)
		return;

	double paddingFactor = 0.9;

	double scalex = screenSize.GetWidth() / worldBox.m_width;
	double scaley = screenSize.GetHeight() / worldBox.m_height;
	m_scale = std::min(scalex, scaley) * paddingFactor;

	double screenw = worldBox.m_width * m_scale;
	double screenh = worldBox.m_height * m_scale;

	double extrax = (screenSize.GetWidth() - screenw) / 2.0;
	double extray = (screenSize.GetHeight() - screenh) / 2.0;

	m_offset = wxPoint2DDouble(-worldBox.m_x + extrax / m_scale,
		-worldBox.m_y + extray / m_scale);
}

wxAffineMatrix2D lsViewControl::GetWorldToScreenMatrix() const
{
	wxAffineMatrix2D mat;
	mat.Translate(m_offset.m_x, m_offset.m_y);
	mat.Scale(m_scale, m_scale);
	return mat;
}

wxAffineMatrix2D lsViewControl::GetScreenToWorldMatrix() const
{
	wxAffineMatrix2D inv = GetWorldToScreenMatrix();
	inv.Invert();
	return inv;
}


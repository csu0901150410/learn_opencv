#pragma once

#include <wx/wx.h>

class lsViewControl
{
public:
	double m_scale = 1.0;// screen / world
	wxPoint2DDouble m_offset = {0, 0};// 世界坐标原点在屏幕中的位置

	wxPoint2DDouble WorldToScreen(const wxPoint2DDouble& pt) const;
	wxPoint2DDouble ScreenToWorld(const wxPoint2DDouble& pt) const;

	void Zoom(double factor, const wxPoint2DDouble& anchor_screen);

	void Pan(double dx, double dy);

	void ZoomToFit(const wxRect2DDouble& worldBox, const wxSize& screenSize);
};

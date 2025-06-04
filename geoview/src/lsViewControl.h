#pragma once

#include <wx/wx.h>

#include "lsPoint.h"

class lsBoundbox;

class lsViewControl
{
public:
	double m_scale = 1.0;// screen / world
	lsPoint m_offset = {0, 0};// 世界坐标原点在屏幕中的位置

	lsPoint WorldToScreen(const lsPoint& pt) const;
	lsPoint ScreenToWorld(const lsPoint& pt) const;

	void Zoom(double factor, const lsPoint& anchor_screen);

	void Pan(double dx, double dy);

	void ZoomToFit(const lsBoundbox& worldBox, const wxSize& screenSize);

	wxAffineMatrix2D GetWorldToScreenMatrix() const;
	wxAffineMatrix2D GetScreenToWorldMatrix() const;

	double GetZoomFactor() const;
};

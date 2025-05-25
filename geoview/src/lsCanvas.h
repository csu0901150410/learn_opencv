#pragma once

#include <wx/wx.h>
#include <wx/docview.h>

/**
 * 画布类。图形绘制区域，处理鼠标事件，绘制文档内容。
 */
class lsCanvas : public wxScrolledWindow
{
public:
	lsCanvas(wxView* view, wxWindow* parent = nullptr);
	virtual ~lsCanvas();

	void SetView(wxView* view);
	void ResetView();

	void FitToWorld(const wxRect2DDouble& worldBox);

private:
	virtual void OnDraw(wxDC& dc);
	virtual void OnPaint(wxPaintEvent& event);
	virtual void OnSize(wxSizeEvent& event);
	void OnMouseEvent(wxMouseEvent& event);

	wxPoint WorldToScreen(const wxPoint2DDouble& pt) const;
	wxPoint2DDouble ScreenToWorld(const wxPoint& pt) const;

	void DrawLine(wxDC& dc, const wxPoint2DDouble& s, const wxPoint2DDouble& e);

private:
	wxView* m_view;

	double m_scale;
	wxPoint2DDouble m_offset;

	wxSize m_lastClientSize;
};

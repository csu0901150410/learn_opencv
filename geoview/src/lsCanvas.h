#pragma once

#include <wx/wx.h>
#include <wx/docview.h>

class lsCanvas : public wxScrolledWindow
{
public:
	lsCanvas(wxView* view, wxWindow* parent = nullptr);
	virtual ~lsCanvas();

	virtual void OnDraw(wxDC& dc);

	virtual void OnPaint(wxPaintEvent& event);

	void SetView(wxView* view);
	void ResetView();

private:
	void OnMouseEvent(wxMouseEvent& event);

private:
	wxView* m_view;
};

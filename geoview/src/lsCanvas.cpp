#include <wx/dcbuffer.h>

#include "lsCanvas.h"

lsCanvas::lsCanvas(wxView* view, wxWindow* parent /*= nullptr*/)
	: wxScrolledWindow(parent ? parent : view->GetFrame())
	, m_view(view)
{
	SetCursor(wxCursor(wxCURSOR_HAND));
	SetBackgroundStyle(wxBG_STYLE_PAINT);

	Bind(wxEVT_PAINT, &lsCanvas::OnPaint, this);
}

lsCanvas::~lsCanvas()
{

}

void lsCanvas::OnDraw(wxDC& dc)
{
	if (m_view)
		m_view->OnDraw(&dc);
	else
	{
		dc.SetBackground(*wxGREY_BRUSH);
		dc.Clear();
	}
}

void lsCanvas::OnPaint(wxPaintEvent& event)
{
	wxAutoBufferedPaintDC dc(this);
	OnDraw(dc);
}

void lsCanvas::SetView(wxView* view)
{
	m_view = view;
}

void lsCanvas::ResetView()
{
	m_view = nullptr;
}

void lsCanvas::OnMouseEvent(wxMouseEvent& event)
{
	
}


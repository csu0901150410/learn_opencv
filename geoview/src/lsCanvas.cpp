#include <wx/dcbuffer.h>

#include "lsCanvas.h"
#include "lsDocument.h"

lsCanvas::lsCanvas(wxView* view, wxWindow* parent /*= nullptr*/)
	: wxScrolledWindow(parent ? parent : view->GetFrame())
	, m_view(view)
	, m_scale(1.0)
{
	SetCursor(wxCursor(wxCURSOR_HAND));
	SetBackgroundStyle(wxBG_STYLE_PAINT);

	Bind(wxEVT_PAINT, &lsCanvas::OnPaint, this);
	Bind(wxEVT_SIZE, &lsCanvas::OnSize, this);
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
	dc.SetBackground(*wxBLACK_BRUSH);
	dc.Clear();

	if (!m_view)
		return;

	auto doc = wxDynamicCast(m_view->GetDocument(), lsDocument);
	if (!doc)
		return;

	if (!doc->GetSegments().empty())
	{
		dc.SetPen(*wxWHITE_PEN);
		for (const auto& seg : doc->GetSegments())
		{
			DrawLine(dc, seg.s, seg.e);
		}
	}

	//OnDraw(dc);
}

void lsCanvas::OnSize(wxSizeEvent& event)
{
	wxSize newSize = GetClientSize();

	if (m_lastClientSize.IsFullySpecified() && m_scale > 0.0)
	{
		// 计算窗口中心点在世界坐标中对应的位置
		auto oldCenterScreen = wxPoint(m_lastClientSize.GetWidth() / 2, 
									   m_lastClientSize.GetHeight() / 2);
		wxPoint2DDouble centerWorld = ScreenToWorld(oldCenterScreen);

		// 计算新的偏移量，让这个点仍然在新窗口中心
		wxPoint newCenterScreen(newSize.GetWidth() / 2,
							    newSize.GetHeight() / 2);
		wxPoint2DDouble newOffset = centerWorld -
			wxPoint2DDouble(newCenterScreen.x / m_scale,
							newCenterScreen.y / m_scale);

		m_offset = -newOffset;
	}

	m_lastClientSize = newSize;

	event.Skip();
	Refresh();
}

void lsCanvas::SetView(wxView* view)
{
	m_view = view;
}

void lsCanvas::ResetView()
{
	m_view = nullptr;
}

void lsCanvas::FitToWorld(const wxRect2DDouble& worldBox)
{
	wxSize clientSize = GetClientSize();

	double paddingFactor = 0.9;

	double scalex = clientSize.GetWidth() / worldBox.m_width;
	double scaley = clientSize.GetHeight() / worldBox.m_height;
	m_scale = std::min(scalex, scaley) * paddingFactor;

	double screenw = worldBox.m_width * m_scale;
	double screenh = worldBox.m_height * m_scale;

	double extrax = (clientSize.GetWidth() - screenw) / 2.0;
	double extray = (clientSize.GetHeight() - screenh) / 2.0;

	m_offset = wxPoint2DDouble(-worldBox.m_x + extrax / m_scale,
		-worldBox.m_y + extray / m_scale);

	Refresh();
}

void lsCanvas::OnMouseEvent(wxMouseEvent& event)
{
	
}

wxPoint lsCanvas::WorldToScreen(const wxPoint2DDouble& pt) const
{
	double x = (pt.m_x + m_offset.m_x) * m_scale;
	double y = (pt.m_y + m_offset.m_y) * m_scale;
	return wxPoint(static_cast<int>(x), static_cast<int>(y));
}

wxPoint2DDouble lsCanvas::ScreenToWorld(const wxPoint& pt) const
{
	return wxPoint2DDouble(pt.x / m_scale - m_offset.m_x,
						   pt.y / m_scale - m_offset.m_y);
}

void lsCanvas::DrawLine(wxDC& dc, const wxPoint2DDouble& s, const wxPoint2DDouble& e)
{
	dc.DrawLine(WorldToScreen(s), WorldToScreen(e));
}

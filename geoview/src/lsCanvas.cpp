#include <wx/dcbuffer.h>
#include <wx/graphics.h>

#include "lsCanvas.h"
#include "lsDocument.h"

lsCanvas::lsCanvas(wxView* view, wxWindow* parent /*= nullptr*/, lsRenderer* renderer /*= nullptr*/)
	: wxScrolledWindow(parent ? parent : view->GetFrame())
	, m_view(view)
	, m_scale(1.0)
	, m_renderer(renderer)
{
	SetCursor(wxCursor(wxCURSOR_HAND));
	SetBackgroundStyle(wxBG_STYLE_PAINT);

	Bind(wxEVT_PAINT, &lsCanvas::OnPaint, this);
	Bind(wxEVT_SIZE, &lsCanvas::OnSize, this);
}

lsCanvas::~lsCanvas()
{
	//delete m_renderer;
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
		// See https://zetcode.com/gfx/cairo/cairobackends/
		cairo_surface_t* surface;
		cairo_t* cr;

		surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 400, 400);
		cr = cairo_create(surface);

		cairo_set_source_rgb(cr, 0, 0, 0);
		cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
		cairo_set_font_size(cr, 40.0);

		cairo_move_to(cr, 10.0, 50.0);
		cairo_show_text(cr, "Hello World");

		// 写到exe文件夹
		cairo_surface_write_to_png(surface, "../build/cairo.png");

		cairo_destroy(cr);
		cairo_surface_destroy(surface);
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

	auto* cairoRenderer = dynamic_cast<lsCairoRenderer*>(m_renderer);
	cairoRenderer->Resize(newSize.GetWidth(), newSize.GetHeight());

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

void lsCanvas::SetRenderer(lsRenderer* renderer)
{
	m_renderer = renderer;
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


lsCairoRenderer::lsCairoRenderer(int width, int height)
{
	Resize(width, height);
}

lsCairoRenderer::~lsCairoRenderer()
{
	if (m_cr)
		cairo_destroy(m_cr);
	if (m_surface)
		cairo_surface_destroy(m_surface);
}

void lsCairoRenderer::BeginDraw()
{
	// 初始化为黑色
	cairo_save(m_cr);
	cairo_set_source_rgba(m_cr, 0, 0, 0, 1);
	cairo_paint(m_cr);
}

void lsCairoRenderer::EndDraw()
{
	cairo_restore(m_cr);
}

void lsCairoRenderer::SetLineWidth(double width)
{
	cairo_set_line_width(m_cr, width);
}

void lsCairoRenderer::SetColor(const wxColour& color)
{
	cairo_set_source_rgba(m_cr, 
		color.Red() / 255.0,
		color.Green() / 255.0,
		color.Blue() / 255.0,
		color.Alpha() / 255.0);
}

void lsCairoRenderer::DrawLine(double sx, double sy, double ex, double ey)
{
	cairo_move_to(m_cr, sx, sy);
	cairo_line_to(m_cr, ex, ey);
	cairo_stroke(m_cr);
}

void lsCairoRenderer::Resize(int width, int height)
{
	if (m_cr)
		cairo_destroy(m_cr);
	if (m_surface)
		cairo_surface_destroy(m_surface);

	// 重新创建绘图surface
	m_width = width;
	m_height = height;
	m_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
	m_cr = cairo_create(m_surface);
}

cairo_surface_t* lsCairoRenderer::GetSurface() const
{
	return m_surface;
}

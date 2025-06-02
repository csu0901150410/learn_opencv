#include <wx/dcbuffer.h>
#include <wx/graphics.h>

#include "lsCanvas.h"
#include "lsDocument.h"
#include "lsEntity.h"

lsCanvas::lsCanvas(wxView* view, wxWindow* parent /*= nullptr*/, lsRenderer* renderer /*= nullptr*/)
	: wxScrolledWindow(parent ? parent : view->GetFrame())
	, m_view(view)
	, m_renderer(renderer)
{
	SetCursor(wxCursor(wxCURSOR_HAND));
	SetBackgroundStyle(wxBG_STYLE_PAINT);

	Bind(wxEVT_PAINT, &lsCanvas::OnPaint, this);
	Bind(wxEVT_MOUSEWHEEL, &lsCanvas::OnMouse, this);
	Bind(wxEVT_RIGHT_DOWN, &lsCanvas::OnMouse, this);
	Bind(wxEVT_RIGHT_UP, &lsCanvas::OnMouse, this);
	Bind(wxEVT_LEFT_DOWN, &lsCanvas::OnMouse, this);
	Bind(wxEVT_LEFT_UP, &lsCanvas::OnMouse, this);
	Bind(wxEVT_MIDDLE_DOWN, &lsCanvas::OnMouse, this);
	Bind(wxEVT_MIDDLE_UP, &lsCanvas::OnMouse, this);
	Bind(wxEVT_MOTION, &lsCanvas::OnMouse, this);
	Bind(wxEVT_SIZE, &lsCanvas::OnSize, this);
	Bind(wxEVT_MOUSE_CAPTURE_LOST, &lsCanvas::OnCaptureLost, this);
	//Bind(wxEVT_LEFT_DOWN, &lsCanvas::OnLeftDown, this);
}

lsCanvas::~lsCanvas()
{

}

void lsCanvas::OnPaint(wxPaintEvent& event)
{
	wxPaintDC dc(this);
	UpdateBitmap();

	if (m_cachedBitmap.IsOk())
		dc.DrawBitmap(m_cachedBitmap, 0, 0);
	else
	{
		dc.SetBackground(*wxBLACK_BRUSH);
		dc.Clear();
	}

	// 画辅助线
	if (m_boxsel)
	{
		dc.SetPen(wxPen(*wxWHITE, 2));
		dc.SetBrush(*wxTRANSPARENT_BRUSH);

		{
			int minx = std::min(m_boxselStartPos.x, m_boxselEndPos.x);
			int miny = std::min(m_boxselStartPos.y, m_boxselEndPos.y);
			int maxx = std::max(m_boxselStartPos.x, m_boxselEndPos.x);
			int maxy = std::max(m_boxselStartPos.y, m_boxselEndPos.y);

			wxPoint lt(minx, miny);
			wxPoint rb(maxx, maxy);
			wxRect rect(lt, rb);
			dc.DrawRectangle(rect);
		}
	}
}

void lsCanvas::OnMouse(wxMouseEvent& event)
{
	if (event.GetWheelRotation() != 0)
	{
		// 滚轮缩放
		double zoomFactor = (event.GetWheelRotation() > 0) ? 1.1 : 1.0 / 1.1;
		m_viewControl.Zoom(zoomFactor, event.GetPosition());

		MakeDirty();
		Refresh();
	}
	else if (event.RightDown() || event.MiddleDown())
	{
		// 右键/中键拖拽
		m_dragging = true;
		m_lastMousePos = event.GetPosition();
		CaptureMouse();
	}
	else if (m_dragging && (event.RightUp() || event.MiddleUp()))
	{
		// 拖拽释放
		m_dragging = false;
		if (HasCapture())
			ReleaseCapture();
	}
	else if (m_dragging && event.Dragging())
	{
		// 拖拽过程
		wxPoint pos = event.GetPosition();
		wxPoint delta = pos - m_lastMousePos;
		m_viewControl.Pan(delta.x, delta.y);
		m_lastMousePos = pos;

		MakeDirty();
		Refresh();
	}
	else if (event.LeftDown())
	{
		m_boxsel = true;
		m_boxselStartPos = event.GetPosition();
		m_boxselEndPos = event.GetPosition();
		CaptureMouse();
	}
	else if (m_boxsel && event.Dragging() && event.LeftIsDown())
	{
		m_boxselEndPos = event.GetPosition();
		Refresh();
	}
	else if (event.LeftUp())
	{
		if (m_boxsel)
		{
			m_boxsel = false;
			m_boxselEndPos = event.GetPosition();
			if (HasCapture())
				ReleaseCapture();

			PerformBoxSelection();

			MakeDirty();
			Refresh();
		}
	}
}

void lsCanvas::OnSize(wxSizeEvent& event)
{
	wxSize newSize = GetClientSize();

	auto* cairoRenderer = dynamic_cast<lsCairoRenderer*>(m_renderer);
	cairoRenderer->Resize(newSize.GetWidth(), newSize.GetHeight());

	MakeDirty();

	event.Skip();
}

void lsCanvas::OnCaptureLost(wxMouseCaptureLostEvent& event)
{
	m_dragging = false;
}

void lsCanvas::OnLeftDown(wxMouseEvent& event)
{
	wxPoint pos = event.GetPosition();
	wxPoint2DDouble worldPos = m_viewControl.ScreenToWorld(pos);

	auto doc = wxDynamicCast(m_view->GetDocument(), lsDocument);
	if (!doc)
		return;

	const double pickTol = 5.0 / m_viewControl.GetZoomFactor();
	bool bMultipleSelMode = event.ControlDown();

	// 遍历数据的行为，最好放到document内部
	bool bHit = false;
	for (auto& entity : doc->GetEntities())
	{
		if (entity->HitTest(worldPos, pickTol))
		{
			if (bMultipleSelMode)
				entity->SetSelected(true);
			else
			{
				// 单选模式，取消其他选择，再选中当前
				doc->DeselectAll();
				entity->SetSelected(true);
			}

			bHit = true;
			break;
		}
	}

	// 单选模式，点击空白区域，取消所有选择
	if (!bMultipleSelMode && !bHit)
	{
		doc->DeselectAll();
	}

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

void lsCanvas::SetRenderer(lsRenderer* renderer)
{
	m_renderer = renderer;
}

void lsCanvas::ZoomToFit()
{
	if (!m_view)
		return;

	auto doc = wxDynamicCast(m_view->GetDocument(), lsDocument);
	if (!doc)
		return;

	m_viewControl.ZoomToFit(doc->GetBoundbox(), GetClientSize());

	MakeDirty();

	Refresh();
}

void lsCanvas::PerformBoxSelection()
{
	wxPoint2DDouble ps = m_viewControl.ScreenToWorld(m_boxselStartPos);
	wxPoint2DDouble pe = m_viewControl.ScreenToWorld(m_boxselEndPos);

	wxRect2DDouble box(
		std::min(ps.m_x, pe.m_x),
		std::min(ps.m_y, pe.m_y),
		std::abs(ps.m_x - pe.m_x),
		std::abs(ps.m_y - pe.m_y)
	);

	auto doc = wxDynamicCast(m_view->GetDocument(), lsDocument);
	if (!doc)
		return;

	if (!wxGetKeyState(WXK_CONTROL))
		doc->DeselectAll();

	for (auto& entity : doc->GetEntities())
	{
		if (entity->IntersectWith(box))
			entity->SetSelected(true);
	}
}

void lsCanvas::MakeDirty()
{
	m_bDirty = true;
}

void lsCanvas::UpdateBitmap()
{
	if (!m_bDirty || !IsShown())
		return;

	if (!m_view)
		return;

	auto doc = wxDynamicCast(m_view->GetDocument(), lsDocument);
	if (!doc)
		return;

	// 重新生成缓存位图
	if (!doc->IsEmpty())
	{
		m_renderer->BeginDraw();

		m_renderer->SetTransform(m_viewControl.GetWorldToScreenMatrix());
		doc->Draw(*m_renderer);

		/*if (m_boxsel)
		{
			wxRect2DDouble box(
				std::min(m_boxselStartPos.x, m_boxselEndPos.x),
				std::min(m_boxselStartPos.y, m_boxselEndPos.y),
				std::abs(m_boxselStartPos.x - m_boxselEndPos.x),
				std::abs(m_boxselStartPos.y - m_boxselEndPos.y)
			);

			m_renderer->SetColor(*wxGREEN);
			m_renderer->DrawRectangle(box.m_x, box.m_y, box.m_width, box.m_height);
			m_renderer->SetColor(*wxRED);
		}*/

		m_renderer->EndDraw();

		m_cachedBitmap = m_renderer->ToBitmap();
		m_bDirty = false;
	}
}

lsCairoRenderer::lsCairoRenderer(wxWindow* window, int width, int height)
	: m_window(window)
{
	Resize(width, height);
}

lsCairoRenderer::~lsCairoRenderer()
{
	release_buffer();
}

void lsCairoRenderer::BeginDraw()
{
	init_surface();

	// 清屏
	cairo_set_source_rgb(m_cr, 0, 0, 0);
	cairo_rectangle(m_cr, 0, 0, m_width, m_height);
	cairo_fill(m_cr);
}

void lsCairoRenderer::EndDraw()
{
	// 绘制结束，将cairo绘制的数据组织成wxImage的格式
	int stride = m_stride;
	int width = m_width;
	int height = m_height;
	unsigned char* src = m_crImageBuffer;
	unsigned char* dst = m_wxImageBuffer;

	// 逐行转换
	for (int row = 0; row < height; ++row)
	{
		// 行内按四通道排列，逐通道转换
		for (int col = 0; col < stride; col += 4)
		{
			const unsigned char* chanel = src + col;

			// src BGRA --> dst RGB
			dst[0] = chanel[2];
			dst[1] = chanel[1];
			dst[2] = chanel[0];

			// next dst chanel
			dst += 3;
		}

		// next src row
		src += stride;
	}

	// 渲染到具体的设备上
	/*wxImage image(width, height, m_wxImageBuffer, true);
	wxBitmap bitmap(image);
	wxMemoryDC mdc(bitmap);
	wxClientDC cdc(m_window);
	cdc.Blit(0, 0, width, height, &mdc, 0, 0, wxCOPY);*/

	// 写到exe文件夹
	if (0)
	{
		cairo_surface_write_to_png(m_surface, "../build/cairo.png");
	}

	deinit_surface();
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
	auto ps = m_matrix.TransformPoint({sx, sy});
	auto pe = m_matrix.TransformPoint({ex, ey});
	cairo_move_to(m_cr, ps.m_x, ps.m_y);
	cairo_line_to(m_cr, pe.m_x, pe.m_y);
	cairo_stroke(m_cr);
}

void lsCairoRenderer::DrawLine(const wxPoint2DDouble& s, const wxPoint2DDouble& e)
{
	DrawLine(s.m_x, s.m_y, e.m_x, e.m_y);
}

void lsCairoRenderer::DrawRectangle(double x, double y, double w, double h)
{
	cairo_rectangle(m_cr, x, y, w, h);
	cairo_stroke(m_cr);
}

void lsCairoRenderer::Resize(int width, int height)
{
	m_width = width;
	m_height = height;

	// 绘图尺寸变了，释放原有绘图缓冲区并重新申请
	release_buffer();
	allocate_buffer();
}

void lsCairoRenderer::SetTransform(const wxAffineMatrix2D& mat)
{
	m_matrix = mat;
}

void lsCairoRenderer::SetHighlight(bool on)
{
	if (on)
		SetColor(*wxCYAN);
	else
		SetColor(*wxRED);
}

cairo_surface_t* lsCairoRenderer::GetSurface() const
{
	return m_surface;
}

wxBitmap lsCairoRenderer::ToBitmap() const
{
	if (!m_wxImageBuffer)
		return wxBitmap();

	wxImage image(m_width, m_height, m_wxImageBuffer, true);
	return wxBitmap(image);
}

void lsCairoRenderer::init_surface()
{
	if (m_initialized)
		return;

	m_surface = cairo_image_surface_create_for_data(m_crImageBuffer,
		CAIRO_FORMAT_ARGB32, m_width, m_height, m_stride);
	m_cr = cairo_create(m_surface);

	m_initialized = true;
}

void lsCairoRenderer::deinit_surface()
{
	if (!m_initialized)
		return;

	cairo_destroy(m_cr);
	m_cr = nullptr;
	cairo_surface_destroy(m_surface);
	m_surface = nullptr;

	m_initialized = false;
}

void lsCairoRenderer::release_buffer()
{
	delete[] m_crImageBuffer;
	m_crImageBuffer = nullptr;
	delete[] m_wxImageBuffer;
	m_wxImageBuffer = nullptr;
}

void lsCairoRenderer::allocate_buffer()
{
	m_stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, m_width);

	// cairo实际每行要对齐到某个宽度，可能和width不一致，stride已经考虑了通道数
	m_crBufferSize = m_stride * m_height;

	// wxImage缓冲区大小
	m_wxBufferSize = m_width * m_height * 3;

	assert(nullptr == m_crImageBuffer);
	m_crImageBuffer = new unsigned char[m_crBufferSize];
	assert(nullptr == m_wxImageBuffer);
	m_wxImageBuffer = new unsigned char[m_wxBufferSize];
}

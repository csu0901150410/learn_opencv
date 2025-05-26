#pragma once

#include <wx/wx.h>
#include <wx/docview.h>

#include "cairo.h"

class lsRenderer
{
public:
	virtual ~lsRenderer() = default;

	virtual void BeginDraw() = 0;
	virtual void EndDraw() = 0;

	virtual void SetLineWidth(double width) = 0;
	virtual void SetColor(const wxColour& color) = 0;

	virtual void DrawLine(double sx, double sy, double ex, double ey) = 0;

	virtual void Resize(int width, int height) = 0;
};

class lsCairoRenderer : public lsRenderer
{
public:
	lsCairoRenderer(int width, int height);
	~lsCairoRenderer();

	void BeginDraw() override;
	void EndDraw() override;

	void SetLineWidth(double width) override;
	void SetColor(const wxColour& color) override;
	
	void DrawLine(double sx, double sy, double ex, double ey) override;

	void Resize(int width, int height) override;

	cairo_surface_t* GetSurface() const;

private:
	cairo_surface_t* m_surface = nullptr;
	cairo_t* m_cr = nullptr;
	int m_width = 0;
	int m_height = 0;
};

/**
 * 画布类。图形绘制区域，处理鼠标事件，绘制文档内容。
 */
class lsCanvas : public wxScrolledWindow
{
public:
	lsCanvas(wxView* view, wxWindow* parent = nullptr, lsRenderer* renderer = nullptr);
	virtual ~lsCanvas();

	void SetView(wxView* view);
	void ResetView();

	void FitToWorld(const wxRect2DDouble& worldBox);

	void SetRenderer(lsRenderer* renderer);

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

	lsRenderer* m_renderer;

	double m_scale;
	wxPoint2DDouble m_offset;

	wxSize m_lastClientSize;
};

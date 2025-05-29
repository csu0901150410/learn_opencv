#pragma once

#include <wx/wx.h>
#include <wx/docview.h>

#include "cairo.h"

#include "lsViewControl.h"

class lsRenderer
{
public:
	virtual ~lsRenderer() = default;

	virtual void BeginDraw() = 0;
	virtual void EndDraw() = 0;

	virtual void SetLineWidth(double width) = 0;
	virtual void SetColor(const wxColour& color) = 0;

	virtual void DrawLine(double sx, double sy, double ex, double ey) = 0;
	virtual void DrawLine(const wxPoint2DDouble& s, const wxPoint2DDouble& e);

	virtual void Resize(int width, int height) = 0;
};

class lsCairoRenderer : public lsRenderer
{
public:
	lsCairoRenderer(wxWindow* window, int width, int height);
	~lsCairoRenderer();

	void BeginDraw() override;
	void EndDraw() override;

	void SetLineWidth(double width) override;
	void SetColor(const wxColour& color) override;
	
	void DrawLine(double sx, double sy, double ex, double ey) override;

	void Resize(int width, int height) override;

	cairo_surface_t* GetSurface() const;

private:
	void init_surface();
	void deinit_surface();

	void release_buffer();
	void allocate_buffer();

private:
	wxWindow* m_window;

	bool m_initialized = false;// 标记cairo对象是否创建，保证不重复创建和释放
	cairo_surface_t* m_surface = nullptr;
	cairo_t* m_cr = nullptr;
	int m_width = 0;
	int m_height = 0;
	int m_stride = 0;
	int m_crBufferSize = 0;
	int m_wxBufferSize = 0;

	double m_lineWidth = 1.0;
	wxColour m_color = *wxRED;

	unsigned char* m_crImageBuffer = nullptr;// cairo绘图缓冲区，四通道
	unsigned char* m_wxImageBuffer = nullptr;// wxImage转换缓冲区，三通道
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

	void SetRenderer(lsRenderer* renderer);

	void ZoomToFit();

private:
	virtual void OnPaint(wxPaintEvent& event);
	virtual void OnMouse(wxMouseEvent& event);
	virtual void OnSize(wxSizeEvent& event);

	void OnCaptureLost(wxMouseCaptureLostEvent& event);

private:
	wxView* m_view;

	lsRenderer* m_renderer;

	lsViewControl m_viewControl;
	bool m_dragging = false;
	wxPoint m_lastMousePos;

	wxSize m_lastClientSize;
};

#include <wx/wx.h>
#include <wx/nativewin.h>
#include "sciter-x.h"
#include "sciter-x-behavior.h"
#include "sciter-x-dom.hpp"

class wxOcvWindow : public wxPanel
{
public:
	wxOcvWindow(wxWindow* parent)
		: wxPanel(parent, wxID_ANY)
	{
		SetBackgroundStyle(wxBG_STYLE_PAINT);

		Bind(wxEVT_PAINT, &wxOcvWindow::OnPaint, this);
	}

	void update_content()
	{
		Refresh();
	}

private:
	void OnPaint(wxPaintEvent& event)
	{
		wxPaintDC dc(this);

		wxSize size = GetClientSize();

		dc.SetBrush(wxBrush(wxColour(255, 0, 0)));
		dc.DrawRectangle(0, 0, size.x, size.y);

		dc.SetBrush(wxBrush(wxColour(0, 0, 255)));
		dc.DrawRectangle(40, 40, size.x - 80, size.y - 80);
	}
};

struct native_ocvwindow_wx : public sciter::event_handler
{
	wxOcvWindow* window = nullptr;
	wxWindow* parentWindow = nullptr;

	// ctor
	native_ocvwindow_wx()
		: window(nullptr)
		, parentWindow(nullptr)
	{
	}

	virtual ~native_ocvwindow_wx()
	{
	}

	virtual bool subscription(HELEMENT he, UINT& event_groups) override
	{
		event_groups = HANDLE_DRAW | HANDLE_SIZE;
		return true;
	}

	virtual void attached(HELEMENT he) override
	{
		HWND sciterHwnd = sciter::dom::element(he).get_element_hwnd(true);

		parentWindow = new wxNativeWindow();
		parentWindow->SetHWND((WXHWND)sciterHwnd);
		parentWindow->AdoptAttributesFromHWND();

		window = new wxOcvWindow(parentWindow);
		window->Show();
	}

	virtual void detached(HELEMENT he) override
	{
		// 释放wxWidgets持有的资源
		if (window)
		{
			window->Destroy();
			window = nullptr;
		}
		if (parentWindow)
		{
			parentWindow->SetHWND(nullptr);
			parentWindow->Destroy();
			parentWindow = nullptr;
		}

		// 释放sciter持有的资源
		asset_release();
	}

	virtual bool handle_draw(HELEMENT he, DRAW_PARAMS& params) override
	{
		if (DRAW_CONTENT != params.cmd || !window)
			return false;

		window->update_content();
		return true;
	}

	virtual void handle_size(HELEMENT he) override
	{
		sciter::dom::element el = sciter::dom::element(he);
		RECT rc = el.get_location(ROOT_RELATIVE | CONTENT_BOX);
		window->SetSize(rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);
		window->update_content();
	}
};

struct native_ocvwindow_wx_factory : public sciter::behavior_factory
{
	native_ocvwindow_wx_factory()
		: behavior_factory("wxOcvWindow")
	{
	}

	virtual sciter::event_handler* create(HELEMENT he) override
	{
		return new native_ocvwindow_wx();
	}
};

native_ocvwindow_wx_factory native_ocvwindow_wx_factory_instance;

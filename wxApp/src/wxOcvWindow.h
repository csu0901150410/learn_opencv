#pragma once

#include <wx/wx.h>
#include <wx/nativewin.h>
#include "sciter-x.h"
#include "sciter-x-behavior.h"
#include "sciter-x-dom.hpp"

#include <opencv2/opencv.hpp>
#include <mutex>
#include <wx/dcbuffer.h>

#include "convertmattowxbmp.h"

#include "wxSciterControl.h"

// 挂载到sciter元素上的wxWidgets窗口
class wxOcvWindow : public wxPanel
{
public:
	wxOcvWindow(wxWindow* parent, HELEMENT he)
		: wxPanel(parent, wxID_ANY)
		, he(he)
		, control(nullptr)
	{
		SetBackgroundStyle(wxBG_STYLE_PAINT);

		Bind(wxEVT_PAINT, &wxOcvWindow::OnPaint, this);
		Bind(wxEVT_RIGHT_DOWN, &wxOcvWindow::OnMouse, this);
	}

	~wxOcvWindow()
	{
		// 销毁时反注册
		if (control)
			control->unregister_ocvwindow(this);
	}

	void set_control(wxSciterControl* wnd)
	{
		control = wnd;
	}

	void update_content()
	{
		Refresh();
	}

	void set_bitmap(const wxBitmap& bitmap)
	{
		// 很危险，可能这里没拷贝完，相机线程那边就写入了
		prepareBitmap = bitmap;
		Refresh();
	}

private:
	void OnPaint(wxPaintEvent& event)
	{
		if (!prepareBitmap.IsOk())
			return;

		wxBufferedPaintDC dc(this);
		PrepareDC(dc);

		dc.SetBackground(*wxBLACK_BRUSH);
		dc.Clear();

		wxSize clientSize = GetClientSize();

		// 居中绘制，不缩放
		int x = (clientSize.GetWidth() - prepareBitmap.GetWidth()) / 2;
		int y = (clientSize.GetHeight() - prepareBitmap.GetHeight()) / 2;

		dc.DrawBitmap(prepareBitmap, x, y, false);
	}

	void OnMouse(wxMouseEvent& event)
	{
		if (wxEVT_RIGHT_DOWN == event.GetEventType())
		{
			if (he)
			{
				// 向该元素发送右键事件
				//SciterPostEvent(he, CONTEXT_MENU_REQUEST, he, CLICK_REASON::BY_MOUSE_CLICK);

				// 向父级窗口发送原始鼠标事件
				sciter::dom::element self = sciter::dom::element::element(he);
				HELEMENT root = self.root();

				sciter::dom::element parent = sciter::dom::element::element(root);

				HWND sciterHwnd = parent.get_element_hwnd(true);
				if (!sciterHwnd)
					return;

				wxPoint globalPt = ::wxGetMousePosition();
				POINT pt = { globalPt.x, globalPt.y };
				::ScreenToClient(sciterHwnd, &pt);

				LPARAM lParam = MAKELPARAM(pt.x, pt.y);
				WPARAM wParam = MK_RBUTTON;

				::PostMessage(sciterHwnd, WM_RBUTTONDOWN, wParam, lParam);
			}
		}
	}

	void render_image(wxDC& dc)
	{

	}

private:
	HELEMENT he;
	wxSciterControl* control;

	wxBitmap prepareBitmap;
};

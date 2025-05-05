#include <wx/wx.h>
#include <wx/nativewin.h>
#include "sciter-x.h"
#include "sciter-x-behavior.h"
#include "sciter-x-dom.hpp"

#include <opencv2/opencv.hpp>
#include <mutex>

#include "convertmattowxbmp.h"

#ifdef WINDOWS
#ifndef THIS_HINSTANCE
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define THIS_HINSTANCE ((HINSTANCE)&__ImageBase)
#endif
#endif

struct lsOcvImageModel
{
	cv::Mat original;

	std::mutex mtx;
	
	// 不同的图像处理过程
	cv::Mat get_processed_view(int viewid)
	{
		std::lock_guard<std::mutex> lock(mtx);
		cv::Mat result;

		switch (viewid)
		{
		case 0:
		{
			result = original.clone();
		}
		break;

		case 1:
		{
			cv::cvtColor(original, result, cv::COLOR_BGR2GRAY);
		}
		break;

		default:
			result = original.clone();
		}

		return result;
	}
};

typedef std::shared_ptr<lsOcvImageModel> lsOcvImageModelPtr;

lsOcvImageModelPtr create_object()
{
	return std::make_shared<lsOcvImageModel>();
}

class wxOcvWindow : public wxPanel
{
public:
	wxOcvWindow(wxWindow* parent, lsOcvImageModelPtr model, int viewid, HELEMENT he)
		: wxPanel(parent, wxID_ANY)
		, model(model)
		, viewid(viewid)
		, he(he)
	{
		SetBackgroundStyle(wxBG_STYLE_PAINT);

		Bind(wxEVT_PAINT, &wxOcvWindow::OnPaint, this);
		Bind(wxEVT_RIGHT_DOWN, &wxOcvWindow::OnMouse, this);
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

		/*dc.SetBrush(wxBrush(wxColour(255, 0, 0)));
		dc.DrawRectangle(0, 0, size.x, size.y);

		dc.SetBrush(wxBrush(wxColour(0, 0, 255)));
		dc.DrawRectangle(40, 40, size.x - 80, size.y - 80);*/

		dc.SetBrush(*wxBLACK_BRUSH);
		dc.DrawRectangle(0, 0, size.x, size.y);

		render_image(dc);
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

	wxBitmap mat2bitmap(const cv::Mat& mat) const
	{
		if (mat.empty())
			return wxBitmap();

		// wxBitmap的宽高应该初始化为和Mat一致，且深度为24
		wxBitmap bitmap(mat.cols, mat.rows, 24);
		bool bSuccess = ConvertMatBitmapTowxBitmap(mat, bitmap);
		if (!bSuccess)
			return wxBitmap();

		// 返回wxBitmap
		return bitmap;
	}

	void render_image(wxDC& dc)
	{
		if (!model)
			return;

		cv::Mat img = model->get_processed_view(viewid);
		if (img.empty())
			return;

		wxBitmap bmp = mat2bitmap(img);

		wxSize clientSize = dc.GetSize();
		wxSize bmpSize = bmp.GetSize();

		int posX = (clientSize.GetWidth() - bmpSize.GetWidth()) / 2;
		int posY = (clientSize.GetHeight() - bmpSize.GetHeight()) / 2;

		dc.DrawBitmap(bmp, posX, posY, false);
	}

private:
	lsOcvImageModelPtr model;
	int viewid;
	HELEMENT he;
};

struct native_ocvwindow_wx : public sciter::event_handler
{
	wxOcvWindow* window = nullptr;
	wxWindow* parentWindow = nullptr;
	lsOcvImageModelPtr model = nullptr;

	// ctor
	native_ocvwindow_wx()
		: window(nullptr)
		, parentWindow(nullptr)
		, model(nullptr)
	{
		model = create_object();
		model->original = cv::imread("../resources/images/wxWidgets-logo.png");
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
		// 这里的关键问题在于，往sciter元素挂载wxWidgets窗口的时候，需要先把wxWidgets
		// 窗口创建出来，这需要指定父窗口指针，但是这时候只能通过元素获取父窗口的句柄，那么
		// 就需要根据父窗口的句柄表示得到wxWidgets窗口指针表示。
		// 目前的方案，使用了windows系统实现类中的SetHWND方法，并不通用。

		HWINDOW sciterHwnd = sciter::dom::element(he).get_element_hwnd(true);

		// 根据元素句柄创建一个能够包含其他wx窗口的窗口作为父窗口
		if (!parentWindow)
		{
			parentWindow = new wxNativeWindow();
			parentWindow->SetHWND((WXHWND)sciterHwnd);
		}

		// 根据元素父窗口创建ocv子窗口
		window = new wxOcvWindow(parentWindow, model, 0, he);
		if (window)
		{
			window->SetSize(parentWindow->GetClientSize());
			window->Show();

			sciter::dom::element self = sciter::dom::element::element(he);
			self.attach_hwnd((HWINDOW)window->GetHandle());
		}
	}

	virtual void detached(HELEMENT he) override
	{
		dispose();
	}

	void dispose()
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

		if (window)
		{
			window->update_content();
		}
		return true;
	}

	virtual void handle_size(HELEMENT he) override
	{
		if (window)
		{
			sciter::dom::element el = sciter::dom::element(he);
			RECT rc = el.get_location(ROOT_RELATIVE | CONTENT_BOX);
			window->SetSize(rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);
			window->update_content();
		}
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

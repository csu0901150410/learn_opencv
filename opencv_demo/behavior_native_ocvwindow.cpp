#include "stdafx.h"
#include "sciter-x.h"
#include "sciter-x-behavior.h"
#include "sciter-x-graphics.hpp"

#ifdef WINDOWS
#ifndef THIS_HINSTANCE
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define THIS_HINSTANCE ((HINSTANCE)&__ImageBase)
#endif
#endif

using namespace std;

static const TCHAR CLASS_NAME[] = TEXT("opencv.demo.clsname");
static bool classRegistered = false;

namespace sciter
{
#ifdef WINDOWS

    // 自定义窗口，用于将窗口嵌入到html元素中
    struct native_ocvwindow : public event_handler
    {
        HWND     this_hwnd;
        HELEMENT this_element;
        // ctor
        native_ocvwindow() : this_hwnd(0), this_element(0) {}
        virtual ~native_ocvwindow() {}

        virtual bool subscription(HELEMENT he, UINT& event_groups)
        {
            // 只处理绘制事件
            event_groups = HANDLE_DRAW;
            return true;
        }

        virtual void attached(HELEMENT he)
        {
			// 窗口附加在元素上

            this_element = he;

            dom::element self = dom::element::element(this_element);

            RECT rc = self.get_location(VIEW_RELATIVE | CONTENT_BOX);

            HWND parent = self.get_element_hwnd(false);

            if (!classRegistered)
            {
				// 注册窗口
                WNDCLASS wndclass = {0};
				wndclass.style = 0;
				wndclass.cbClsExtra = 0;
				wndclass.cbWndExtra = 0;
				wndclass.hInstance = THIS_HINSTANCE;
				wndclass.hIcon = NULL;
				wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
				wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
				wndclass.lpszMenuName = NULL;
				wndclass.lpszClassName = CLASS_NAME;
				wndclass.lpfnWndProc = window_procedure;// 要指定窗口过程函数，否则创建失败
				if (!::RegisterClass(&wndclass))
				{
					return;
				}
                classRegistered = true;
            }


            this_hwnd = ::CreateWindow(
                CLASS_NAME,
                TEXT(""),
                WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE,
                rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
                parent, NULL, THIS_HINSTANCE, this);

            ::SendMessage(this_hwnd, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), true);

            self.attach_hwnd(this_hwnd);
        }
        virtual void detached(HELEMENT he)
        {
            if (this_hwnd)
                ::DestroyWindow(this_hwnd);
            asset_release();
        }

		virtual bool handle_draw(HELEMENT he, DRAW_PARAMS& params)
		{
			// drawing only content layer
			if( params.cmd != DRAW_CONTENT )
				return false;

			/*unsigned w = unsigned(params.area.right - params.area.left);
			unsigned h = unsigned(params.area.bottom - params.area.top);
			do_draw(GetDC(this_hwnd), w, h);*/

            // 通知窗口绘制
			if (this_hwnd)
            {
				//::InvalidateRect(this_hwnd, nullptr, TRUE); // 标记整个窗口为脏区
				//::UpdateWindow(this_hwnd);                  // 立即发送WM_PAINT

                // 只能用InvalidateRect将重绘消息放入消息队列，UpdateWindow会跳过
                // 消息队列直接到窗口过程，造成重绘竞争
                ::InvalidateRect(this_hwnd, nullptr, TRUE);
			}

			return true;
		}

		void do_draw(HDC hdc, unsigned width, unsigned height) {
			RECT rc = { 0,0,int(width),int(height) };
			HBRUSH hbr1 = ::CreateSolidBrush(0xFF0000);
			HBRUSH hbr2 = ::CreateSolidBrush(0x0000FF);
			// fill it by black color:
			::FillRect(hdc, &rc, hbr1);
			::InflateRect(&rc, -40, -40);
			::FillRect(hdc, &rc, hbr2);
			::DeleteObject(hbr1);
			::DeleteObject(hbr2);
		}

    private:
        static LRESULT CALLBACK window_procedure(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
        {
            native_ocvwindow* pThis = nullptr;

            if (WM_NCCREATE == msg)
            {
                CREATESTRUCT* pStruct = (CREATESTRUCT*)lParam;
                pThis = (native_ocvwindow*)pStruct->lpCreateParams;
                SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
            }
            else
            {
                pThis = (native_ocvwindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
            }

            if (pThis)
            {
                pThis->event_loop(msg, wParam, lParam);
            }

            return DefWindowProc(hwnd, msg, wParam, lParam);
        }

        // 消息循环
        LRESULT event_loop(UINT msg, WPARAM wParam, LPARAM lParam)
        {
            switch (msg)
            {
            case WM_PAINT:
            {
				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(this_hwnd, &ps);

				unsigned w = unsigned(ps.rcPaint.right - ps.rcPaint.left);
				unsigned h = unsigned(ps.rcPaint.bottom - ps.rcPaint.top);
				do_draw(GetDC(this_hwnd), w, h);

                EndPaint(this_hwnd, &ps);
            }
            break;
            }

            return DefWindowProc(this_hwnd, msg, wParam, lParam);
        }
    };
#endif
    struct native_ocvwindow_factory : public behavior_factory {

        native_ocvwindow_factory() : behavior_factory("native-ocvwindow") { }

        // the only behavior_factory method:
        virtual event_handler* create(HELEMENT he) {
#ifdef WINDOWS
            return new native_ocvwindow();
#else 
            return nullptr;
#endif
        }

    };

    // instantiating and attaching it to the global list
    native_ocvwindow_factory native_ocvwindow_factory_instance;


}

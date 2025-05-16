#include "lsApp.h"

// for wxGetInstance()
#ifdef __WINDOWS__
#include <wx/msw/private.h>
#endif

#include "lsMainFrame.h"

HHOOK g_hKeyboardHook = nullptr;

LRESULT CALLBACK KeyboardProc(int code, WPARAM wParam, LPARAM lParam)
{
	// 表示事件需要钩子函数处理
	if (code == HC_ACTION)
	{
		// 获取当前的焦点窗口，向其发送键盘事件
		HWND hwndFocus = GetFocus();
		if (hwndFocus && IsWindow(hwndFocus))
		{
			TCHAR className[256] = { 0 };
			GetClassName(hwndFocus, className, sizeof(className) / sizeof(TCHAR));

			// 只处理名为 "H-SMILE-FRAME" 的 Sciter 弹出窗口
			if (_tcscmp(className, _T("H-SMILE-FRAME")) == 0)
			{
				if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)
				{
					KBDLLHOOKSTRUCT* pkb = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);

					BYTE keyboardState[256];
					if (!GetKeyboardState(keyboardState))
						return CallNextHookEx(g_hKeyboardHook, code, wParam, lParam);

					WCHAR unicodeChar[2] = { 0 };
					UINT scanCode = pkb->scanCode;
					HKL layout = GetKeyboardLayout(0);

					int result = ToUnicodeEx(
						pkb->vkCode,
						scanCode,
						keyboardState,
						unicodeChar,
						2,
						0,
						layout
					);

					if (result > 0)
					{
						// 不能用Post把消息放到消息队列
						::SendMessage(hwndFocus, WM_CHAR, unicodeChar[0], 1);
						return 0;
					}
				}
			}
		}
	}

	// 继续调用链
	return CallNextHookEx(g_hKeyboardHook, code, wParam, lParam);
}

// 安装键盘钩子函数
void InstallKeyboardHook()
{
	if (!g_hKeyboardHook)
	{
		// 全局键盘钩子函数
		g_hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, wxGetInstance(), 0);
	}
}

// 卸载键盘钩子函数
void UninstallKeyboardHook()
{
	if (g_hKeyboardHook)
	{
		UnhookWindowsHookEx(g_hKeyboardHook);
		g_hKeyboardHook = nullptr;
	}
}

bool lsApp::OnInit()
{
	InstallKeyboardHook();

	SciterClassName();

	m_mainframe = new lsMainFrame("Sciter integration for wxWidgets", wxPoint(50, 50), wxSize(1080, 600));

	wxIcon icon;
	icon.LoadFile("../resources/images/icons/app.ico", wxBITMAP_TYPE_ICO);
	m_mainframe->SetIcon(icon);

	m_mainframe->Show(TRUE);
	SetTopWindow(m_mainframe);
	return TRUE;
}

int lsApp::OnExit()
{
	UninstallKeyboardHook();
	return wxApp::OnExit();
}

lsMainFrame* lsApp::get_mainframe()
{
	return m_mainframe;
}



std::wstring string_to_wstring(const std::string& str)
{
	if (str.empty())
		return L"";

	// 计算所需缓冲区大小
	int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
	if (len == 0) return L"";

	// 转换
	std::wstring wstr(len, 0);
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstr[0], len);
	return wstr;
}

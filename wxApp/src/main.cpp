#include <wx/wx.h> 
#include <wx/filedlg.h>

#include "sciter-control.h"

#ifdef __WINDOWS__
#include <wx/msw/private.h>
#endif

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

class MyApp : public wxApp
{
	virtual bool OnInit() override;
	virtual int OnExit() override;
};


class MyFrame : public wxFrame
{
public:
	wxSciterControl* m_sciter;

	MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size);
	~MyFrame();

	void OnQuit(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
	void OnOpen(wxCommandEvent& event);
	void OnPaint(wxPaintEvent& event);
};

enum
{
	ID_Quit = 1,
	ID_About,
	ID_Open,
};

IMPLEMENT_APP(MyApp);

bool MyApp::OnInit()
{
	InstallKeyboardHook();

	SciterClassName();
	MyFrame* frame = new MyFrame("Sciter integration for wxWidgets", wxPoint(50, 50), wxSize(1080, 600));

	wxIcon icon;
	icon.LoadFile("../resources/images/icons/app.ico", wxBITMAP_TYPE_ICO);
	frame->SetIcon(icon);

	frame->Show(TRUE);
	SetTopWindow(frame);
	return TRUE;
}

int MyApp::OnExit()
{
	UninstallKeyboardHook();
	return wxApp::OnExit();
}

MyFrame::MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
	: wxFrame((wxFrame*)NULL, -1, title, pos, size)
	, m_sciter()
{
	wxMenu* menuFile = new wxMenu;
	menuFile->Append(ID_Open, "&Open...", "Load html to sciter window.");
	menuFile->Append(ID_About, "&About...");
	menuFile->AppendSeparator();
	menuFile->Append(ID_Quit, "E&xit", "Quit application.");

	wxMenuBar* menuBar = new wxMenuBar;
	menuBar->Append(menuFile, "&File");

	SetMenuBar(menuBar);


	//wxFontSelectorCtrl* ctl = new wxFontSelectorCtrl(this, this->NewControlId());
	m_sciter = new wxSciterControl(this, this->NewControlId());
	//m_sciter->load_file(L"http://httpbin.org/html");
	m_sciter->load_file(L"home://../resources/wxhtml/main.htm");

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(m_sciter, 1, wxEXPAND);
    SetSizer(sizer);
    
    // 应用布局
    Layout();

	CreateStatusBar();
	SetStatusText("Welcome to wxWindows!");

	Centre();

	Bind(wxEVT_MENU, &MyFrame::OnQuit, this, ID_Quit);
	Bind(wxEVT_MENU, &MyFrame::OnOpen, this, ID_Open);
	Bind(wxEVT_MENU, &MyFrame::OnAbout, this, ID_About);
	Bind(wxEVT_PAINT, &MyFrame::OnPaint, this);
}

MyFrame::~MyFrame()
{
	
}

void MyFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
	Close(TRUE);
}

void MyFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
	wxMessageBox("This is a wxWindows + Sciter child control sample", "About", wxOK | wxICON_INFORMATION, this);
}

void MyFrame::OnOpen(wxCommandEvent& event)
{
	wxFileDialog dlg(this);
	dlg.SetWildcard("HTML files|*.htm;*.html|All files|*.*");
	if (dlg.ShowModal() == wxID_CANCEL)
		return;

	const wxString path = "file:///" + dlg.GetPath();
	m_sciter->load_file(path.ToStdWstring().c_str());
	this->SetStatusText(dlg.GetPath());
}

void MyFrame::OnPaint(wxPaintEvent& event)
{
	int a = 100;
}


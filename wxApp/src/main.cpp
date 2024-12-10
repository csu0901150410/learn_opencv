#include <wx/wx.h> 
#include <wx/filedlg.h>

#include "sciter-control.h"

#ifdef __WINDOWS__
#include <wx/msw/private.h>
#endif

class MyApp : public wxApp
{
	virtual bool OnInit();
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

	WXLRESULT MSWWindowProc(WXUINT message, WXWPARAM wParam, WXLPARAM lParam) override
    {
        if (message == WM_NCCALCSIZE)
        {
            if (wParam)
            {
                NCCALCSIZE_PARAMS* params = reinterpret_cast<NCCALCSIZE_PARAMS*>(lParam);
                // 保留标题栏但允许客户区绘制到非客户区
                RECT originalRect = params->rgrc[0];
                LONG originalTop = originalRect.top;
                
                LRESULT result = wxFrame::MSWWindowProc(message, wParam, lParam);
                
                // 恢复顶部位置，这样可以保留标题栏交互功能
                params->rgrc[0].top = originalTop;
                return 0;
            }
        }
        return wxFrame::MSWWindowProc(message, wParam, lParam);
    }

	DECLARE_EVENT_TABLE()
};

enum
{
	ID_Quit = 1,
	ID_About,
	ID_Open,
};

BEGIN_EVENT_TABLE(MyFrame, wxFrame)
EVT_MENU(ID_Quit, MyFrame::OnQuit)
EVT_MENU(ID_About, MyFrame::OnAbout)
EVT_MENU(ID_Open, MyFrame::OnOpen)
END_EVENT_TABLE();

IMPLEMENT_APP(MyApp);

bool MyApp::OnInit()
{
	SciterClassName();
	MyFrame* frame = new MyFrame("Sciter integration for wxWidgets", wxPoint(50, 50), wxSize(1920, 1080));
	frame->Show(TRUE);
	SetTopWindow(frame);
	return TRUE;
}

MyFrame::MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
	: wxFrame((wxFrame*)NULL, -1, title, pos, size, (wxDEFAULT_FRAME_STYLE & ~wxBORDER_MASK) | wxCLIP_CHILDREN)
	, m_sciter()
{
	/*wxMenu* menuFile = new wxMenu;
	menuFile->Append(ID_Open, "&Open...", "Load html to sciter window.");
	menuFile->Append(ID_About, "&About...");
	menuFile->AppendSeparator();
	menuFile->Append(ID_Quit, "E&xit", "Quit application.");

	wxMenuBar* menuBar = new wxMenuBar;
	menuBar->Append(menuFile, "&File");

	SetMenuBar(menuBar);*/

	// 设置扩展窗口样式，允许子窗口绘制到非客户区
#ifdef __WINDOWS__
    HWND hwnd = (HWND)GetHandle();
    SetWindowLong(hwnd, GWL_STYLE, 
        GetWindowLong(hwnd, GWL_STYLE) | WS_CLIPCHILDREN);
    // 触发非客户区重新计算
    SetWindowPos(hwnd, NULL, 0, 0, 0, 0, 
        SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
#endif

	//wxFontSelectorCtrl* ctl = new wxFontSelectorCtrl(this, this->NewControlId());
	m_sciter = new wxSciterControl(this, this->NewControlId());
	m_sciter->load_file(L"http://httpbin.org/html");

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(m_sciter, 1, wxEXPAND);
    SetSizer(sizer);
    
    // 应用布局
    Layout();

	/*CreateStatusBar();
	SetStatusText("Welcome to wxWindows!");*/

	Centre();
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
#include "lsMainFrame.h"

lsMainFrame::lsMainFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
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

	Bind(wxEVT_MENU, &lsMainFrame::OnQuit, this, ID_Quit);
	Bind(wxEVT_MENU, &lsMainFrame::OnOpen, this, ID_Open);
	Bind(wxEVT_MENU, &lsMainFrame::OnAbout, this, ID_About);
	Bind(wxEVT_PAINT, &lsMainFrame::OnPaint, this);
}

lsMainFrame::~lsMainFrame()
{

}

void lsMainFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
	Close(TRUE);
}

void lsMainFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
	wxMessageBox("This is a wxWindows + Sciter child control sample", "About", wxOK | wxICON_INFORMATION, this);
}

void lsMainFrame::OnOpen(wxCommandEvent& event)
{
	wxFileDialog dlg(this);
	dlg.SetWildcard("HTML files|*.htm;*.html|All files|*.*");
	if (dlg.ShowModal() == wxID_CANCEL)
		return;

	const wxString path = "file:///" + dlg.GetPath();
	m_sciter->load_file(path.ToStdWstring().c_str());
	this->SetStatusText(dlg.GetPath());
}

void lsMainFrame::OnPaint(wxPaintEvent& event)
{
	int a = 100;
}

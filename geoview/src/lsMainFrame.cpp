#include "lsMainFrame.h"

IMPLEMENT_DYNAMIC_CLASS(lsMainFrame, wxDocParentFrame)

lsMainFrame::lsMainFrame()
{

}

lsMainFrame::lsMainFrame(wxDocManager* manager, wxFrame* parent, wxWindowID id, const wxString& title, const wxPoint& pos /*= wxDefaultPosition*/, const wxSize& size /*= wxDefaultSize*/)
{
	Create(manager, parent, id, title, pos, size);
}

lsMainFrame::~lsMainFrame()
{
	m_auiManager.UnInit();
}

bool lsMainFrame::Create(wxDocManager* manager, wxFrame* parent, wxWindowID id, const wxString& title, const wxPoint& pos /*= wxDefaultPosition*/, const wxSize& size /*= wxDefaultSize*/)
{
	bool res = wxDocParentFrame::Create(manager, parent, id, title, pos, size);
	if (res)
	{
		CreateControls();

		Bind(wxEVT_MENU, &lsMainFrame::OnExit, this, wxID_EXIT);
	}
	return res;
}

void lsMainFrame::OnExit(wxCommandEvent& event)
{
	Close();
}

void lsMainFrame::CreateControls()
{
	SetMenuBar(CreateMenuBar());

	m_auiManager.SetManagedWindow(this);


	m_editor = new wxTextCtrl(this, ID_EDITOR, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);


	//m_auiManager.AddPane(new wxPanel(this, wxID_ANY), wxAuiPaneInfo().CenterPane().Name(_("Canvas")));

	m_auiManager.AddPane(m_editor, wxAuiPaneInfo().CenterPane().Name(_("Editor")));

	m_auiManager.Update();
}

wxMenuBar* lsMainFrame::CreateMenuBar()
{
	wxMenuBar *menuBar = new wxMenuBar;

	wxMenu *fileMenu = new wxMenu;
	fileMenu->Append(wxID_NEW, _("New"));
	fileMenu->Append(wxID_OPEN, _("Open"));
	fileMenu->Append(wxID_SAVE, _("Save"));
	fileMenu->AppendSeparator();
	fileMenu->Append(wxID_EXIT, _("Exit"));

	menuBar->Append(fileMenu, _("File"));

	return menuBar;
}


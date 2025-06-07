#include "lsApp.h"
#include "lsDefs.h"
#include "lsMainFrame.h"
#include "lsDocument.h"
#include "lsView.h"

wxIMPLEMENT_APP(lsApp);

bool lsApp::OnInit()
{
	if (!wxApp::OnInit())
		return false;

	wxLog::AddTraceMask(TRACE_GEOVIEW);

	m_docManager = new wxDocManager;
	m_docManager->SetMaxDocsOpen(1);

	wxDocTemplate *docTemplate = new wxDocTemplate(m_docManager,
		wxT("demo document"), wxT("*.txt;*.gbr;"),
		wxEmptyString, wxT("TXT"), wxT("doc"), wxT("view"),
		CLASSINFO(lsDocument), CLASSINFO(lsView));

	m_mainFrame = new lsMainFrame(m_docManager, nullptr, wxID_ANY, wxT("demo"), wxDefaultPosition, wxSize(800, 600));
	SetTopWindow(m_mainFrame);

	wxIcon icon;
	icon.LoadFile("../resources/images/icons/app.ico", wxBITMAP_TYPE_ICO);
	m_mainFrame->SetIcon(icon);

	m_docManager->CreateNewDocument();

	m_mainFrame->Center();
	m_mainFrame->Show(true);

	return true;
}

int lsApp::OnExit()
{
	wxDELETE(m_docManager);
	return wxApp::OnExit();
}

lsCanvas* lsApp::GetMainWindowCanvas() const
{
	return m_mainFrame->m_canvas;
}


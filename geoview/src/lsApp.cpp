#include "lsApp.h"
#include "lsMainFrame.h"
#include "lsDocument.h"
#include "lsView.h"

IMPLEMENT_APP(lsApp)

bool lsApp::OnInit()
{
	wxLog::AddTraceMask(TRACE_GEOVIEW);

	m_docManager = new wxDocManager;
	m_docManager->SetMaxDocsOpen(1);

	wxDocTemplate *docTemplate = new wxDocTemplate(m_docManager,
		wxT("demo document"), wxT("*.txt;*.gbr;"),
		wxEmptyString, wxT("TXT"), wxT("doc"), wxT("view"),
		CLASSINFO(lsDocument), CLASSINFO(lsView));

	lsMainFrame *frame = new lsMainFrame(m_docManager, nullptr, wxID_ANY, wxT("demo"));
	SetTopWindow(frame);

	wxIcon icon;
	icon.LoadFile("../resources/images/icons/app.ico", wxBITMAP_TYPE_ICO);
	frame->SetIcon(icon);

	frame->Center();
	frame->Show(true);

	return true;
}

int lsApp::OnExit()
{
	wxDELETE(m_docManager);
	return wxApp::OnExit();
}


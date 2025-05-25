#include "lsView.h"
#include "lsDocument.h"
#include "lsMainFrame.h"
#include "lsCanvas.h"

#include "lsApp.h"

wxIMPLEMENT_DYNAMIC_CLASS(lsView, wxView);

lsView::lsView()
	: m_canvas(nullptr)
{
	wxLogTrace(TRACE_GEOVIEW, wxT("lsView::lsView"));
	SetFrame(wxGetApp().GetTopWindow());
}

bool lsView::OnCreate(wxDocument* doc, long flags)
{
	if (!wxView::OnCreate(doc, flags))
		return false;

	m_canvas = wxGetApp().GetMainWindowCanvas();
	m_canvas->SetView(this);
	m_canvas->Refresh();

	return true;
}

void lsView::OnDraw(wxDC* dc)
{
	dc->SetBackground(*wxBLACK_BRUSH);
	dc->Clear();

	wxLogTrace(TRACE_GEOVIEW, wxT("lsView::OnDraw"));
}

void lsView::OnUpdate(wxView* sender, wxObject* hint /*= (wxObject *)nullptr*/)
{
	wxLogTrace(TRACE_GEOVIEW, wxT("lsView::OnUpdate"));
}

bool lsView::OnClose(bool deleteWindow /*= true*/)
{
	wxLogTrace(TRACE_GEOVIEW, wxT("lsView::OnClose"));
	if (!wxView::OnClose(deleteWindow))
		return false;

	Activate(false);

	m_canvas->ClearBackground();
	m_canvas->ResetView();
	m_canvas = nullptr;

	if (GetFrame())
		wxStaticCast(GetFrame(), wxFrame)->SetTitle(wxGetApp().GetAppDisplayName());

	return true;
}

lsDocument* lsView::GetDocument()
{
	return wxStaticCast(wxView::GetDocument(), lsDocument);
}

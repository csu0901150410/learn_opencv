#include "lsView.h"
#include "lsMainFrame.h"

#include "lsApp.h"

IMPLEMENT_DYNAMIC_CLASS(lsView, wxView);

lsView::lsView()
{
	wxLogTrace(TRACE_GEOVIEW, wxT("lsView::lsView"));
	SetFrame(wxTheApp->GetTopWindow());
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
	if (!GetDocument()->Close())
	{
		return false;
	}

	SetFrame(NULL);
	Activate(false);
	return true;
}

wxTextCtrl* lsView::GetEditor()
{
	do
	{
		lsMainFrame *frame = wxDynamicCast(GetFrame(), lsMainFrame);
		if (!frame)
			break;
		return frame->m_editor;
	}
	while (false);
	return nullptr;
}


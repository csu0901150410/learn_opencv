#include "lsDocument.h"
#include "lsView.h"

#include "lsApp.h"

IMPLEMENT_DYNAMIC_CLASS(lsDocument, wxDocument);

lsDocument::lsDocument()
{
	wxLogTrace(TRACE_GEOVIEW, wxT("lsDocument::lsDocument"));
}

bool lsDocument::OnSaveDocument(const wxString& filename)
{
	do 
	{
		lsView *view = (lsView*)GetFirstView();
		if (!view)
			break;
		wxTextCtrl *editor = view->GetEditor();
		if (!editor)
			break;

		if (editor->SaveFile(filename))
		{
			return false;
		}
		Modify(false);
	} while (false);
	return true;
}

bool lsDocument::OnOpenDocument(const wxString& filename)
{
	do
	{
		lsView* view = (lsView*)GetFirstView();
		if (!view)
			break;
		wxTextCtrl* editor = view->GetEditor();
		if (!editor)
			break;

		if (editor->LoadFile(filename))
		{
			return false;
		}
		SetFilename(filename, true);
		Modify(false);
		UpdateAllViews();
	} while (false);
	return true;
}

bool lsDocument::IsModified() const
{
	bool res = wxDocument::IsModified();
	lsView *view = (lsView*)GetFirstView();
	if (view)
	{
		res |= view->GetEditor()->IsModified();
	}
	return res;
}

void lsDocument::Modify(bool mod)
{
	lsView *view = (lsView*)GetFirstView();
	wxDocument::Modify(mod);

	if (!mod && view && view->GetEditor())
	{
		view->GetEditor()->DiscardEdits();
	}
}

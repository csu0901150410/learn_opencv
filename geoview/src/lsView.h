#pragma once

#include <wx/wx.h>
#include <wx/docview.h>

class lsView : public wxView
{
public:
	lsView();

	virtual void OnDraw(wxDC *dc);
	virtual void OnUpdate(wxView *sender, wxObject *hint = (wxObject *)nullptr);
	virtual bool OnClose(bool deleteWindow = true);

public:
	wxTextCtrl *GetEditor();

private:
	DECLARE_DYNAMIC_CLASS(lsView);
};

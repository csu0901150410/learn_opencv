#pragma once

#include <wx/wx.h>
#include <wx/docview.h>

class lsCanvas;
class lsDocument;

class lsView : public wxView
{
public:
	lsView();

	virtual bool OnCreate(wxDocument* doc, long flags);
	virtual void OnDraw(wxDC *dc);
	virtual void OnUpdate(wxView *sender, wxObject *hint = (wxObject *)nullptr);
	virtual bool OnClose(bool deleteWindow = true);

public:
	lsDocument* GetDocument();

private:
	lsCanvas* m_canvas;

	wxDECLARE_DYNAMIC_CLASS(lsView);
};

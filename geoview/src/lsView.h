#pragma once

#include <wx/wx.h>
#include <wx/docview.h>

class lsCanvas;
class lsDocument;

/**
 * 视图类。决定文档的数据如何展示，响应文档更新。
 */
class lsView : public wxView
{
public:
	lsView();

	virtual bool OnCreate(wxDocument* doc, long flags);
	virtual void OnDraw(wxDC *dc);
	virtual void OnUpdate(wxView *sender, wxObject *hint = (wxObject *)nullptr);
	virtual bool OnClose(bool deleteWindow = true);

	void OnGenerateRandomLines(wxCommandEvent& event);

public:
	lsDocument* GetDocument();

private:
	lsCanvas* m_canvas;

	wxDECLARE_DYNAMIC_CLASS(lsView);
};

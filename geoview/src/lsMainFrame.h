#pragma once

#include <wx/wx.h>
#include <wx/docview.h>

class lsCanvas;

class lsMainFrame : public wxDocParentFrame
{
public:
	lsMainFrame();
	lsMainFrame(wxDocManager *manager, wxFrame *parent, wxWindowID id,
		const wxString &title, const wxPoint &pos = wxDefaultPosition,
		const wxSize &size = wxDefaultSize);
	~lsMainFrame();

	bool Create(wxDocManager *manager, wxFrame *parent, wxWindowID id,
		const wxString& title, const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize);

	void OnExit(wxCommandEvent& event);

public:
	enum
	{
		ID_CANVAS = wxID_HIGHEST + 1,
	};

	lsCanvas* m_canvas;

private:
	void CreateControls();
	wxMenuBar *CreateMenuBar();

private:

	wxDECLARE_DYNAMIC_CLASS(lsMainFrame);
};

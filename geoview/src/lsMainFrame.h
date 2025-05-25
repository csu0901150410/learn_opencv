#pragma once

#include <wx/wx.h>
#include <wx/docview.h>
#include <wx/aui/aui.h>

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
		ID_EDITOR = wxID_HIGHEST + 1,
	};

	wxTextCtrl* m_editor;

private:
	void CreateControls();
	wxMenuBar *CreateMenuBar();

private:
	wxAuiManager m_auiManager;

	DECLARE_DYNAMIC_CLASS(lsMainFrame)
};

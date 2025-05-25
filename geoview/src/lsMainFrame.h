#pragma once

#include <wx/wx.h>
#include <wx/docview.h>

class lsCanvas;

enum
{
	ID_CANVAS = wxID_HIGHEST + 1,
	ID_MENU_RANDOM_LINES,
};

/**
 * 主窗口。持有菜单栏、工具栏、状态栏。
 */
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

	lsCanvas* m_canvas;

private:
	void CreateControls();
	wxMenuBar *CreateMenuBar();

private:

	wxDECLARE_DYNAMIC_CLASS(lsMainFrame);
};

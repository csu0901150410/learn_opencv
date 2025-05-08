#pragma once

#include <wx/wx.h>

#include "wxSciterControl.h"

class lsMainFrame : public wxFrame
{
public:
	wxSciterControl* m_sciter;

	lsMainFrame(const wxString& title, const wxPoint& pos, const wxSize& size);
	~lsMainFrame();

	void OnQuit(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
	void OnOpen(wxCommandEvent& event);
	void OnPaint(wxPaintEvent& event);

	enum
	{
		ID_Quit = 1,
		ID_About,
		ID_Open,
	};
};

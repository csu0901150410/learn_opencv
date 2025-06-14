﻿#pragma once

#include <wx/wx.h>
#include <wx/docview.h>

class lsCanvas;

class lsMainFrame;

class lsApp : public wxApp
{
public:
	virtual bool OnInit();
	virtual int OnExit();

	lsCanvas* GetMainWindowCanvas() const;

private:
	wxDocManager *m_docManager;
	lsMainFrame *m_mainFrame;
};

wxDECLARE_APP(lsApp);

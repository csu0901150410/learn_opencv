#pragma once

#include <wx/wx.h>
#include <wx/docview.h>

// for wxLogTrace
#define TRACE_GEOVIEW "GEOVIEW"

class lsApp : public wxApp
{
public:
	virtual bool OnInit();
	virtual int OnExit();

private:
	wxDocManager *m_docManager;
};

wxDECLARE_APP(lsApp);

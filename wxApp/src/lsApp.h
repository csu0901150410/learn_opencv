#pragma once

#include <wx/wx.h>

class lsApp : public wxApp
{
	virtual bool OnInit() override;
	virtual int OnExit() override;
};

IMPLEMENT_APP(lsApp);

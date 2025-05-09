#pragma once

#include <wx/wx.h>

#include "json/json.h"

class lsMainFrame;

class lsApp : public wxApp
{
public:
	lsMainFrame* get_mainframe();

private:
	virtual bool OnInit() override;
	virtual int OnExit() override;

private:
	lsMainFrame* m_mainframe;
};

void call_script(const std::string& name);
void call_script(const std::string& name, const Json::Value& params);

IMPLEMENT_APP(lsApp);

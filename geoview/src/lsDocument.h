#pragma once

#include <wx/wx.h>
#include <wx/docview.h>

class lsDocument : public wxDocument
{
public:
	lsDocument();

	std::ostream& SaveObject(std::ostream& stream);
	std::istream& LoadObject(std::istream& stream);

private:
	wxDECLARE_DYNAMIC_CLASS(lsDocument);
};

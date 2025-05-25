#pragma once

#include <wx/wx.h>
#include <wx/docview.h>

class lsDocument : public wxDocument
{
public:
	lsDocument();

	virtual bool OnSaveDocument(const wxString& filename);
	virtual bool OnOpenDocument(const wxString& filename);
	virtual bool IsModified() const;
	virtual void Modify(bool mod);

private:
	DECLARE_DYNAMIC_CLASS(lsDocument);
};

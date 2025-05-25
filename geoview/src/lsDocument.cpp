#include "lsDocument.h"
#include "lsView.h"

#include "lsApp.h"

wxIMPLEMENT_DYNAMIC_CLASS(lsDocument, wxDocument);

lsDocument::lsDocument()
	: wxDocument()
{
	wxLogTrace(TRACE_GEOVIEW, wxT("lsDocument::lsDocument"));
}

std::ostream& lsDocument::SaveObject(std::ostream& ostream)
{
	return ostream;
}

std::istream& lsDocument::LoadObject(std::istream& istream)
{
	return istream;
}


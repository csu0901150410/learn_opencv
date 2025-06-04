#include "lsDxfReader.h"
#include "lsDocument.h"
#include "lsLine.h"

#include <wx/geometry.h>

bool lsDxfReader::import(lsDocument* document, const std::string& filepath)
{
	m_document = document;

	bool bSuccess = m_dxf.in(filepath.c_str(), this);
	if (!bSuccess)
		return false;
	return true;
}

void lsDxfReader::addLine(const DL_LineData& data)
{
	lsPoint ps, pe;
	ps.x = data.x1;
	ps.y = data.y1;
	pe.x = data.x2;
	pe.y = data.y2;
	m_document->AddEntity(std::make_shared<lsLine>(ps, pe, 1.0));
}


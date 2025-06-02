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
	wxPoint2DDouble ps, pe;
	ps.m_x = data.x1;
	ps.m_y = data.y1;
	pe.m_x = data.x2;
	pe.m_y = data.y2;
	m_document->AddEntity(std::make_shared<lsLine>(ps, pe, 1.0));
}


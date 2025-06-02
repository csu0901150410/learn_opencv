#pragma once

#include "dl_creationadapter.h"
#include "dl_dxf.h"

class lsDocument;

class lsDxfReader : public DL_CreationAdapter
{
public:
	lsDxfReader() = default;

	bool import(lsDocument* document, const std::string& filepath);

	void addLine(const DL_LineData& data) override;

private:
	DL_Dxf m_dxf;
	lsDocument* m_document;
};

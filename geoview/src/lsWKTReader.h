#pragma once

#include <string>

class lsDocument;

class lsWKTReader
{
public:
	lsWKTReader(lsDocument* document, const std::string& filepath);

private:
	void generate_random_segments();

private:
	lsDocument* m_document;
};

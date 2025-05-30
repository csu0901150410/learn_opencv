#pragma once

#include <wx/wx.h>
#include <wx/docview.h>

class lsRenderer;

struct LineSegment
{
	wxPoint2DDouble s;
	wxPoint2DDouble e;
};

/**
 * 文档类。存储实际的数据结构，比如图层、图元。
 */
class lsDocument : public wxDocument
{
public:
	lsDocument();

	std::ostream& SaveObject(std::ostream& stream);
	std::istream& LoadObject(std::istream& stream);

public:
	void Draw(lsRenderer& renderer);

public:
	const std::vector<LineSegment>& GetSegments() const;

	void GenerateRandomLines(int count = 100);

	wxRect2DDouble GetBoundbox() const;

private:
	std::vector<LineSegment> m_segments;

private:
	wxDECLARE_DYNAMIC_CLASS(lsDocument);
};

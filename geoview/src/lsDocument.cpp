#include "lsDocument.h"
#include "lsView.h"

#include "lsApp.h"

#include "lsCanvas.h"// for lsRenderer

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

void lsDocument::Draw(lsRenderer& renderer)
{
	// 定义了entity接口之后，可以把renderer传入entity由其确定如何绘制
	for (const auto& seg : GetSegments())
	{
		renderer.DrawLine(seg.s, seg.e);
	}
}

const std::vector<LineSegment>& lsDocument::GetSegments() const
{
	return m_segments;
}

void lsDocument::GenerateRandomLines(int count /*= 100*/)
{
	double maxx = 1200;
	double maxy = 800;

	m_segments.clear();
	for (int i = 0; i < count; ++i)
	{
		wxPoint2DDouble p1(rand() % static_cast<int>(maxx),
						   rand() % static_cast<int>(maxy));
		wxPoint2DDouble p2(rand() % static_cast<int>(maxx),
						   rand() % static_cast<int>(maxy));
		m_segments.push_back({ p1, p2 });
	}
	Modify(true);            // 标记文档已修改
	UpdateAllViews();        // 通知视图刷新
}

wxRect2DDouble lsDocument::GetBoundbox() const
{
	if (m_segments.empty())
		return wxRect2DDouble(0, 0, 1, 1);

	double minx = m_segments[0].s.m_x;
	double miny = m_segments[0].s.m_y;
	double maxx = minx;
	double maxy = miny;

	for (const auto& seg : m_segments)
	{
		for (const auto& pt : { seg.s, seg.e })
		{
			minx = std::min(minx, pt.m_x);
			miny = std::min(miny, pt.m_y);
			maxx = std::max(maxx, pt.m_x);
			maxy = std::max(maxy, pt.m_y);
		}
	}

	return wxRect2DDouble(minx, miny, maxx - minx, maxy - miny);
}


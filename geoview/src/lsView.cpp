#include "lsView.h"
#include "lsDocument.h"
#include "lsMainFrame.h"
#include "lsCanvas.h"

#include "lsApp.h"

#include "lsLine.h"
#include "lsDxfReader.h"
#include "lsWKTReader.h"

#include <unordered_map>

wxIMPLEMENT_DYNAMIC_CLASS(lsView, wxView);

lsView::lsView()
	: m_canvas(nullptr)
{
	wxLogTrace(TRACE_GEOVIEW, wxT("lsView::lsView"));
	SetFrame(wxGetApp().GetTopWindow());

	// 绑定事件处理函数
	Bind(wxEVT_MENU, &lsView::OnGenerateRandomLines, this, ID_MENU_RANDOM_LINES);
	Bind(wxEVT_MENU, &lsView::OnCanvasZoomToFit, this, ID_MENU_CANVAS_FIT);
	Bind(wxEVT_MENU, &lsView::OnLoadDxfFile, this, ID_MENU_LOAD_DXF);
	Bind(wxEVT_MENU, &lsView::OnTestGeos, this, ID_MENU_TEST_GEOS);
	Bind(wxEVT_MENU, &lsView::OnShowIntersection, this, ID_MENU_SHOW_INTER);
}

bool lsView::OnCreate(wxDocument* doc, long flags)
{
	if (!wxView::OnCreate(doc, flags))
		return false;

	m_canvas = wxGetApp().GetMainWindowCanvas();
	m_canvas->SetView(this);
	m_canvas->Refresh();

	return true;
}

void lsView::OnDraw(wxDC* dc)
{
	dc->SetBackground(*wxBLACK_BRUSH);
	dc->Clear();

	wxLogTrace(TRACE_GEOVIEW, wxT("lsView::OnDraw"));
}

void lsView::OnUpdate(wxView* sender, wxObject* hint /*= (wxObject *)nullptr*/)
{
	wxLogTrace(TRACE_GEOVIEW, wxT("lsView::OnUpdate"));

	auto* doc = wxDynamicCast(GetDocument(), lsDocument);
	if (doc && m_canvas)
	{
		// 触发画布重绘
		m_canvas->MakeDirty();
		m_canvas->Refresh();
	}
}

bool lsView::OnClose(bool deleteWindow /*= true*/)
{
	wxLogTrace(TRACE_GEOVIEW, wxT("lsView::OnClose"));
	if (!wxView::OnClose(deleteWindow))
		return false;

	Activate(false);

	m_canvas->ClearBackground();
	m_canvas->ResetView();
	m_canvas = nullptr;

	if (GetFrame())
		wxStaticCast(GetFrame(), wxFrame)->SetTitle(wxGetApp().GetAppDisplayName());

	return true;
}

void lsView::OnGenerateRandomLines(wxCommandEvent& event)
{
	auto doc = wxDynamicCast(GetDocument(), lsDocument);
	if (doc)
	{
		double maxx = 1200;
		double maxy = 800;

		doc->ClearEntities();
		for (int i = 0; i < 2; ++i)
		{
			lsPoint p1(rand() % static_cast<int>(maxx),
				rand() % static_cast<int>(maxy));
			lsPoint p2(rand() % static_cast<int>(maxx),
				rand() % static_cast<int>(maxy));
			doc->AddEntity(std::make_shared<lsLine>(p1, p2, 1.0));
		}
		doc->Modify(true);            // 标记文档已修改
		doc->UpdateAllViews();        // 通知视图刷新

		m_canvas->ZoomToFit();
	}
}

void lsView::OnCanvasZoomToFit(wxCommandEvent& event)
{
	if (!m_canvas)
		return;
	m_canvas->ZoomToFit();
}

void lsView::OnLoadDxfFile(wxCommandEvent& event)
{
	wxFileDialog dlg(
		wxGetApp().GetTopWindow(),
		wxT("Select DXF File"),
		"",
		"",
		wxT("DXF File (*.dxf)|*.dxf"),
		wxFD_OPEN | wxFD_FILE_MUST_EXIST
	);

	if (wxID_OK == dlg.ShowModal())
	{
		wxString filepath = dlg.GetPath();
		auto doc = wxDynamicCast(GetDocument(), lsDocument);
		if (doc)
		{
			doc->ClearEntities();

			lsDxfReader dxfReader;
			dxfReader.import(doc, filepath.ToStdString());

			doc->Modify(true);            // 标记文档已修改
			doc->UpdateAllViews();        // 通知视图刷新
			m_canvas->ZoomToFit();
		}
	}
}

void lsView::OnTestGeos(wxCommandEvent& event)
{
	//wxFileDialog dlg(
	//	wxGetApp().GetTopWindow(),
	//	wxT("Select DXF File"),
	//	"",
	//	"",
	//	wxT("DXF File (*.dxf)|*.dxf"),
	//	wxFD_OPEN | wxFD_FILE_MUST_EXIST
	//);

	//if (wxID_OK == dlg.ShowModal())
	//{
	//	wxString filepath = dlg.GetPath();
	//	auto doc = wxDynamicCast(GetDocument(), lsDocument);
	//	if (doc)
	//	{
	//		doc->ClearEntities();

	//		lsDxfReader dxfReader;
	//		dxfReader.import(doc, filepath.ToStdString());

	//		doc->Modify(true);            // 标记文档已修改
	//		doc->UpdateAllViews();        // 通知视图刷新
	//		m_canvas->ZoomToFit();
	//	}
	//}

	auto doc = wxDynamicCast(GetDocument(), lsDocument);
	if (doc)
	{
		doc->ClearEntities();

		lsWKTReader wktReader(doc, "");

		doc->Modify(true);            // 标记文档已修改
		doc->UpdateAllViews();        // 通知视图刷新
		m_canvas->ZoomToFit();
	}
}

void lsView::OnShowIntersection(wxCommandEvent& event)
{
	auto doc = wxDynamicCast(GetDocument(), lsDocument);
	if (!doc)
		return;

	std::vector<lsLine> lines;
	const auto& ents = doc->GetEntities();
	std::unordered_map<const lsEntity*, const lsEntity*> calculated;
	for (auto& ent : ents)
	{
		const lsLine* pLine = dynamic_cast<lsLine*>(ent.get());
		if (!pLine)
			continue;
		if (calculated.count(ent.get()))
			continue;

		for (auto& e : ents)
		{
			const lsLine* pSeg = dynamic_cast<lsLine*>(e.get());
			if (!pSeg)
				continue;
			if (pLine == pSeg)
				continue;
			if (calculated.count(e.get()))
				continue;

			// 两个线段计算交点
			lsPoint pos;
			if (pLine->IntersectWith(*pSeg, pos))
			{
				// 相交，画一个叉
				const lsReal radius = 10;

				lsPoint p1(pos.x - radius * std::cos(3.14 / 4), pos.y + radius * std::sin(3.14 / 4));
				lsPoint p2(pos.x + radius * std::cos(3.14 / 4), pos.y - radius * std::sin(3.14 / 4));
				lsPoint q1(pos.x - radius * std::cos(3.14 / 4), pos.y - radius * std::sin(3.14 / 4));
				lsPoint q2(pos.x + radius * std::cos(3.14 / 4), pos.y + radius * std::sin(3.14 / 4));

				lines.push_back(lsLine(p1, p2, 1.0));
				lines.push_back(lsLine(q1, q2, 1.0));

				calculated.insert(std::make_pair(pLine, pSeg));
				calculated.insert(std::make_pair(pSeg, pLine));
			}
		}
	}

	for (auto& line : lines)
	{
		doc->AddEntity(std::make_shared<lsLine>(line));
	}

	doc->Modify(true);            // 标记文档已修改
	doc->UpdateAllViews();        // 通知视图刷新
	m_canvas->ZoomToFit();
}

lsDocument* lsView::GetDocument()
{
	return wxStaticCast(wxView::GetDocument(), lsDocument);
}

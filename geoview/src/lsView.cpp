#include "lsView.h"
#include "lsDocument.h"
#include "lsMainFrame.h"
#include "lsCanvas.h"

#include "lsApp.h"

#include "lsLine.h"
#include "lsDxfReader.h"

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
		for (int i = 0; i < 20000; ++i)
		{
			wxPoint2DDouble p1(rand() % static_cast<int>(maxx),
				rand() % static_cast<int>(maxy));
			wxPoint2DDouble p2(rand() % static_cast<int>(maxx),
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

lsDocument* lsView::GetDocument()
{
	return wxStaticCast(wxView::GetDocument(), lsDocument);
}

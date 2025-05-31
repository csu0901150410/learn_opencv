#include "lsDocument.h"
#include "lsView.h"

#include "lsApp.h"

#include "lsCanvas.h"// for lsRenderer
#include "lsEntity.h"

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
	for (const auto& entity : GetEntities())
	{
		entity->Draw(renderer);
	}
}

bool lsDocument::IsEmpty() const
{
	return m_entities.empty();
}

void lsDocument::ClearEntities()
{
	m_entities.clear();
}

const std::vector<std::shared_ptr<lsEntity>>& lsDocument::GetEntities() const
{
	return m_entities;
}

void lsDocument::AddEntity(std::shared_ptr<lsEntity> entity)
{
	m_entities.push_back(entity);
}

wxRect2DDouble lsDocument::GetBoundbox() const
{
	wxRect2DDouble box(0, 0, 1, 1);
	if (m_entities.empty())
		return box;

	for (const auto& entity : m_entities)
	{
		box.Union(entity->GetBoundBox());
	}

	return box;
}

void lsDocument::SelectAll()
{
	for (auto& entity : m_entities)
		entity->SetSelected(true);
}

void lsDocument::DeselectAll()
{
	for (auto& entity : m_entities)
		entity->SetSelected(false);
}


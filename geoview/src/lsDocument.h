#pragma once

#include <wx/wx.h>
#include <wx/docview.h>

class lsRenderer;
class lsEntity;

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

	bool IsEmpty() const;

	void ClearEntities();

	const std::vector<std::shared_ptr<lsEntity>>& GetEntities() const;

	void AddEntity(std::shared_ptr<lsEntity> entity);

	wxRect2DDouble GetBoundbox() const;

	void SelectAll();
	void DeselectAll();

private:

	std::vector<std::shared_ptr<lsEntity>> m_entities;

private:
	wxDECLARE_DYNAMIC_CLASS(lsDocument);
};

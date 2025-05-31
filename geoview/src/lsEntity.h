#pragma once

class lsRenderer;

class lsEntity
{
public:
	virtual ~lsEntity() = default;

	virtual void Draw(lsRenderer& renderer) const = 0;

	virtual wxRect2DDouble GetBoundBox() const = 0;

	virtual bool HitTest(const wxPoint2DDouble& pos, double tol) const = 0;

	void SetSelected(bool sel) { m_selected = sel; }
	bool IsSelected() const { return m_selected; }

protected:
	bool m_selected = false;
};

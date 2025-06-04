#pragma once

class lsRenderer;
class lsPoint;
class lsBoundbox;

class lsEntity
{
public:
	virtual ~lsEntity() = default;

	virtual void Draw(lsRenderer& renderer) const = 0;

	virtual lsBoundbox GetBoundBox() const = 0;

	virtual bool HitTest(const lsPoint& pos, double tol) const = 0;

	void SetSelected(bool sel) { m_selected = sel; }
	bool IsSelected() const { return m_selected; }

	virtual bool IntersectWith(const lsBoundbox& box) const = 0;

protected:
	bool m_selected = false;
};

#pragma once

class lsRenderer;

class lsEntity
{
public:
	virtual ~lsEntity() = default;

	virtual void Draw(lsRenderer& renderer) const = 0;

	virtual wxRect2DDouble GetBoundBox() const = 0;
};

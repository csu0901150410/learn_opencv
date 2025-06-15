#pragma once

#include <memory>

#include <geos/geom/GeometryFactory.h>

class lsLine;
class lsPoint;

class lsGeosAdapter
{
public:
	static std::unique_ptr<geos::geom::Geometry> seg2geos(const lsLine& seg);
	static lsLine geos2seg(const geos::geom::Geometry* geom);

	static bool intersection(const lsPoint& p1, const lsPoint& p2,
		const lsPoint& q1, const lsPoint& q2, lsPoint& intersect);

private:
	static geos::geom::GeometryFactory::Ptr m_factory;
};

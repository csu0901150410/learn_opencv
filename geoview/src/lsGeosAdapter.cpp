#include "lsGeosAdapter.h"

#include "lsLine.h"

#include <geos/algorithm/Intersection.h>

using namespace geos::geom;
using namespace geos::algorithm;

GeometryFactory::Ptr lsGeosAdapter::m_factory = GeometryFactory::create();

std::unique_ptr<geos::geom::Geometry> lsGeosAdapter::seg2geos(const lsLine& seg)
{
	auto coords = std::make_unique<CoordinateSequence>();
	coords->add(Coordinate(seg.s.x, seg.s.y));
	coords->add(Coordinate(seg.e.x, seg.e.y));

	return std::unique_ptr<Geometry>(m_factory->createLineString(std::move(coords)));
}

lsLine lsGeosAdapter::geos2seg(const geos::geom::Geometry* geom)
{
	const auto& coords = geom->getCoordinates();
	lsPoint ps(coords->getAt(0).x, coords->getAt(0).y);
	lsPoint pe(coords->getAt(1).x, coords->getAt(1).y);
	return lsLine(ps, pe, 1);
}

bool lsGeosAdapter::intersection(const lsPoint& p1, const lsPoint& p2, 
	const lsPoint& q1, const lsPoint& q2, lsPoint& intersect)
{
	CoordinateXY p = Intersection::intersection(CoordinateXY(p1.x, p1.y),
												CoordinateXY(p2.x, p2.y),
												CoordinateXY(q1.x, q1.y),
												CoordinateXY(q2.x, q2.y));
	if (p.isNull())
		return false;
	intersect = lsPoint(p.x, p.y);
	return true;
}

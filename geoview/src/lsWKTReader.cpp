#include <random>

#include <geos/geom/GeometryFactory.h>

#include "lsWKTReader.h"
#include "lsDocument.h"
#include "lsLine.h"

lsWKTReader::lsWKTReader(lsDocument* document, const std::string& filepath)
	: m_document(document)
{
	generate_random_segments();
}

void lsWKTReader::generate_random_segments()
{
	int count = 10;
	double xmin = 0;
	double xmax = 400;
	double ymin = 0;
	double ymax = 300;

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> disX(xmin, xmax);
	std::uniform_int_distribution<> disY(ymin, ymax);

	std::vector<std::shared_ptr<geos::geom::Geometry>> segments;

	for (int i = 0; i < count; ++i)
	{
		double x1 = disX(gen);
		double y1 = disY(gen);
		double x2 = disX(gen);
		double y2 = disY(gen);

		using namespace geos::geom;
		const GeometryFactory* factory = GeometryFactory::getDefaultInstance();
		auto coords = std::make_unique<CoordinateSequence>();
		coords->add(CoordinateXY(x1, y1));
		coords->add(CoordinateXY(x2, y2));
		segments.emplace_back(factory->createLineString(std::move(coords)));
	}

	// 转成线段
	for (auto ptr : segments)
	{
		const geos::geom::LineString* seg = dynamic_cast<const geos::geom::LineString*>(ptr.get());
		if (!seg)
			continue;

		const auto& coords = seg->getCoordinates();
		lsPoint ps(coords->getAt(0).x, coords->getAt(0).y);
		lsPoint pe(coords->getAt(1).x, coords->getAt(1).y);
		m_document->AddEntity(std::make_shared<lsLine>(ps, pe, 1.0));
	}
}


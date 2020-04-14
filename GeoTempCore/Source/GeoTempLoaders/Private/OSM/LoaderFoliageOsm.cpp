#include "OSM/LoaderFoliageOsm.h"


void ULoaderFoliageOsm::SetOsmReader_Implementation(UOsmReader* inOsmReader)
{
	osmReader = inOsmReader;
}


TArray<FContourData> ULoaderFoliageOsm::GetFolliage_Implementation()
{
	TArray<FContourData> polygons;

	polygons.Empty();
	//find all building and building parts through ways
	for (auto wayP : osmReader->Ways)
	{
		auto way = wayP.second;
		auto FoliageIterNatural = way->Tags.Find("natural");
		auto FoliageIterLanduse = way->Tags.Find("landuse");
		auto FoliageIterLeisure = way->Tags.Find("leisure");

		FContourData polygon;
		//if this is building or part
		if	(	FoliageIterNatural && FoliageIterNatural->Equals("wood")
			||	FoliageIterLanduse && FoliageIterLanduse->Equals("forest")
			||	FoliageIterLeisure && (FoliageIterLeisure->Equals("park") || FoliageIterLeisure->Equals("garden"))
			)
		{

			//get all points of this way
			TArray<FVector> points;
			points.Reserve(way->Nodes.size());
			for (auto node : way->Nodes)
			{
				points.Add(node->Point);
			}

			auto cont = FContour(points);
			polygon.Outer.Add(cont);

			polygon.Tags = way->Tags;
			polygon.ZeroLat = osmReader->GeoCoords.ZeroLat;
			polygon.ZeroLon = osmReader->GeoCoords.ZeroLon;

			polygons.Add(polygon);
		}
	}

	for (auto relationP : osmReader->Relations)
	{
		auto relation = relationP.second;
		auto FoliageIterNatural = relation->Tags.Find("natural");
		auto FoliageIterLanduse = relation->Tags.Find("landuse");
		auto FoliageIterLeisure = relation->Tags.Find("leisure");

		//if this relation is building
		if	(	FoliageIterNatural && FoliageIterNatural->Equals("wood")
			||	FoliageIterLanduse && FoliageIterLanduse->Equals("forest")
			||	FoliageIterLeisure && (FoliageIterLeisure->Equals("park") || FoliageIterLeisure->Equals("garden"))
			)
		{
			FContourData polygon;

			//now iterate over the ways in this relation
			for (auto element : relation->WayRoles)
			{
				auto way = relation->Ways[element.first];
				if (!way)
				{
					continue;
				}

				auto contour = FContour();
				for (auto node : way->Nodes)
				{
					contour.Points.Add(node->Point);
				}

				if (element.second == "outer")
				{
					contour.FixClockwise();
					polygon.Outer.Add(contour);
				}
				else if (element.second == "inner")
				{
					contour.FixClockwise(true);
					polygon.Holes.Add(contour);
				}
			}

			polygon.Tags = relation->Tags;
			polygon.ZeroLat = osmReader->GeoCoords.ZeroLat;
			polygon.ZeroLon = osmReader->GeoCoords.ZeroLon;

			polygons.Add(polygon);
		}
	}
	return polygons;
}

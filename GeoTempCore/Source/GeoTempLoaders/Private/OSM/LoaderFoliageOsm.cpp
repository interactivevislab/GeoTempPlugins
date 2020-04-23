#include "OSM/LoaderFoliageOsm.h"


void ULoaderFoliageOsm::SetOsmReader_Implementation(UOsmReader* inOsmReader)
{
	osmReader = inOsmReader;
}


TArray<FMultipolygonData> ULoaderFoliageOsm::GetFolliage_Implementation()
{
	TArray<FMultipolygonData> polygons;

	polygons.Empty();
	//find all building and building parts through ways
	for (auto wayP : osmReader->Ways)
	{
		auto way = wayP.Value;
		auto FoliageIterNatural = way->Tags.Find("natural");
		auto FoliageIterLanduse = way->Tags.Find("landuse");
		auto FoliageIterLeisure = way->Tags.Find("leisure");

		FMultipolygonData polygon;
		//if this is building or part
		if	(	FoliageIterNatural && FoliageIterNatural->Equals("wood")
			||	FoliageIterLanduse && FoliageIterLanduse->Equals("forest")
			||	FoliageIterLeisure && (FoliageIterLeisure->Equals("park") || FoliageIterLeisure->Equals("garden"))
			)
		{

			//get all points of this way
			TArray<FVector> points;
			points.Reserve(way->Nodes.Num());
			for (auto node : way->Nodes)
			{
				points.Add(node->Point);
			}

			auto cont = FContour(points);
			polygon.Outer.Add(cont);

			polygon.Tags = way->Tags;
			polygon.Origin = osmReader->GeoCoords;

			polygons.Add(polygon);
		}
	}

	for (auto relationP : osmReader->Relations)
	{
		auto relation = relationP.Value;
		auto FoliageIterNatural = relation->Tags.Find("natural");
		auto FoliageIterLanduse = relation->Tags.Find("landuse");
		auto FoliageIterLeisure = relation->Tags.Find("leisure");

		//if this relation is building
		if	(	FoliageIterNatural && FoliageIterNatural->Equals("wood")
			||	FoliageIterLanduse && FoliageIterLanduse->Equals("forest")
			||	FoliageIterLeisure && (FoliageIterLeisure->Equals("park") || FoliageIterLeisure->Equals("garden"))
			)
		{
			FMultipolygonData polygon;

			//now iterate over the ways in this relation
			for (auto element : relation->WayRoles)
			{
				auto way = relation->Ways.Find(element.Key);
				if (!way)
				{
					continue;
				}

				auto contour = FContour();
				for (auto node : (*way)->Nodes)
				{
					contour.Points.Add(node->Point);
				}

				if (element.Value == "outer")
				{
					contour.FixClockwise();
					polygon.Outer.Add(contour);
				}
				else if (element.Value == "inner")
				{
					contour.FixClockwise(true);
					polygon.Holes.Add(contour);
				}
			}

			polygon.Tags = relation->Tags;
			polygon.Origin = osmReader->GeoCoords;

			polygons.Add(polygon);
		}
	}
	return polygons;
}

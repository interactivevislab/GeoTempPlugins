#include "OSM/LoaderFoliageOsm.h"

inline void FixFoliageContours(FContourData& polygon)
{
	for (auto& cont : polygon.Outer)
	{
		cont.FixLoop();
		cont.FixClockwise();
	}

	for (auto& cont : polygon.Holes)
	{
		cont.FixLoop();
		cont.FixClockwise(true);
	}
}


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
		OsmRelation* relation = relationP.second;
		auto FoliageIterNatural = relation->Tags.Find("natural");
		auto FoliageIterLanduse = relation->Tags.Find("landuse");
		FString* FoliageIterLeisure = relation->Tags.Find("leisure");

		//if this relation is building
		if	(	FoliageIterNatural && FoliageIterNatural->Equals("wood")
			||	FoliageIterLanduse && FoliageIterLanduse->Equals("forest")
			||	FoliageIterLeisure && (FoliageIterLeisure->Equals("park") || FoliageIterLeisure->Equals("garden"))
			)
		{
			FContourData polygon;

			//now iterate over the ways in this relation
			for (std::pair<long, std::string> element : relation->WayRoles)
			{
				auto way = relation->Ways[element.first];
				if (!way)
				{
					continue;
				}

				auto contour = FContour();
				for (OsmNode* node : way->Nodes)
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
	for (auto& polygon : polygons)
	{
		FixFoliageContours(polygon);
	}
	return polygons;
}


inline const FString* FindFoliageTag(TMap<FString, FString> inTags, FString inTag, FString inTagPrefix = "building:")
{
	auto tag = inTags.Find(inTagPrefix + inTag);
	if (!tag)
	{
		tag = inTags.Find(inTag);
	}
	return tag;
}

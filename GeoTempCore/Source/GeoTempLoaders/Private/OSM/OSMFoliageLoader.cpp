#include "OSM/OsmFoliageLoader.h"

inline void FixFoliageContours(FPosgisContourData& polygon)
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


void UOsmFoliageLoader::SetOsmReader_Implementation(UOsmReader* inOsmReader)
{
	OsmReader = inOsmReader;
}


TArray<FPosgisContourData> UOsmFoliageLoader::GetFoliage()
{
	TArray<FPosgisContourData> polygons;

	polygons.Empty();
	//find all building and building parts through ways
	for (auto wayP : OsmReader->Ways)
	{
		auto way = wayP.second;
		auto FoliageIterNatural = way->Tags.Find("natural");
		auto FoliageIterLanduse = way->Tags.Find("landuse");
		auto FoliageIterLeisure = way->Tags.Find("leisure");

		FPosgisContourData polygon;
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
			polygon.ZeroLat = OsmReader->GeoCoords.ZeroLat;
			polygon.ZeroLon = OsmReader->GeoCoords.ZeroLon;

			polygons.Add(polygon);
		}
	}

	for (auto relationP : OsmReader->Relations)
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
			FPosgisContourData polygon;

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
			polygon.ZeroLat = OsmReader->GeoCoords.ZeroLat;
			polygon.ZeroLon = OsmReader->GeoCoords.ZeroLon;

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

#include "OSM/LoaderFoliageOsm.h"

#include "LoaderHelper.h"


void ULoaderFoliageOsm::SetOsmReader_Implementation(UOsmReader* inOsmReader)
{
	osmReader = inOsmReader;
}


TArray<FMultipolygonData> ULoaderFoliageOsm::GetFolliage_Implementation()
{
	TArray<FMultipolygonData> polygons;

	TArray<FContour> UnclosedOuterContours;
	TArray<FContour> UnclosedInnerContours;

	polygons.Empty();
	//find all building and building parts through ways
	for (auto wayP : osmReader->Ways)
	{
		auto way = wayP.Value;
		auto FoliageIterNatural = way->Tags.Find("natural");
		auto FoliageIterLanduse = way->Tags.Find("landuse");
		auto FoliageIterLeisure = way->Tags.Find("leisure");

		bool water = FoliageIterNatural ? FoliageIterNatural->Equals("water") : false;

		auto buildIter = way->Tags.Find("building");
		auto partIter = way->Tags.Find("building:part");

		FMultipolygonData polygon;
		//if this is building or part
		if	(	FoliageIterNatural && FoliageIterNatural->Equals("wood")
			||	FoliageIterLanduse && FoliageIterLanduse->Equals("forest")
			||	FoliageIterLeisure && (FoliageIterLeisure->Equals("park") || FoliageIterLeisure->Equals("garden"))
			||	buildIter || partIter || water
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

			if (buildIter || partIter || water)
			{
				polygon.Tags.Add(TPair<FString, FString>("Type", "Exclude"));
			}
			else
			{
				if (FoliageIterLeisure)
				{
					polygon.Tags.Add(TPair<FString, FString>("typeRole", "park"));
				}
				else
				{
					polygon.Tags.Add(TPair<FString, FString>("typeRole", "forest"));
				}
				auto leafTag = way->Tags.Find("leaf_type");
				if (leafTag)
				{
					polygon.Tags.Add(TPair<FString, FString>("leaf_type", *leafTag));
				}
				else
				{
					polygon.Tags.Add(TPair<FString, FString>("leaf_type", "mixed"));
				}
			}

			polygon.Origin = osmReader->GeoCoords;

			polygons.Add(polygon);
		}

		if (way->Tags.Contains("highway") || way->Tags.Contains("waterway"))
		{
			FVector pointDelta;

			auto lanes = ULoaderHelper::TryGetTag(way->Tags, "lanes", ULoaderHelper::DEFAULT_LANES);
			auto width = ULoaderHelper::TryGetTag(way->Tags, "width", lanes * ULoaderHelper::DEFAULT_LANE_WIDTH);

			for (int i = 0; i < way->Nodes.Num() - 1; i++)
			{
				FMultipolygonData roadPolygon;
				auto startPoint = way->Nodes[i]->Point;
				auto endPoint = way->Nodes[i + 1]->Point;

				pointDelta = FVector::CrossProduct((startPoint - endPoint).GetSafeNormal(), FVector(0, 0, 1));

				pointDelta *= (width * 50);

				auto point0 = startPoint + pointDelta;
				auto point1 = startPoint - pointDelta;
				auto point2 = endPoint + pointDelta;
				auto point3 = endPoint - pointDelta;

				auto roadCont = FContour();
				roadCont.Points.Append({
					point0,
					point2 
				});

				const int capDensity = 8;
				auto yDelta = FVector::CrossProduct(pointDelta, FVector::UpVector);

				for (int j = 1; j < capDensity; j++)
				{
					float angle = PI / capDensity * j;

					float x = FMath::Cos(angle);
					float y = FMath::Sin(angle);

					roadCont.Points.Add(endPoint - FVector(0, 0, 2) + (pointDelta * x + yDelta * y));
				}

				roadCont.Points.Append({
					point3,
					point1
				});

				for (int j = 1; j < capDensity; j++)
				{
					float angle = PI / capDensity * j;

					float x = FMath::Cos(angle);
					float y = FMath::Sin(angle);

					roadCont.Points.Add(startPoint - FVector(0, 0, 2) - (pointDelta * x + yDelta * y));
				}
				
				roadCont.Points.Append({
					point0
				});
				
				roadPolygon.Outer.Add(roadCont);
				roadPolygon.Tags = way->Tags;
				roadPolygon.Tags.Add(TPair<FString, FString>("Type", "Exclude"));

				polygons.Add(roadPolygon);
			}
		}
	}

	for (auto relationP : osmReader->Relations)
	{
		auto relation = relationP.Value;
		auto FoliageIterNatural = relation->Tags.Find("natural");
		auto FoliageIterLanduse = relation->Tags.Find("landuse");
		auto FoliageIterLeisure = relation->Tags.Find("leisure");

		bool water = FoliageIterNatural ? FoliageIterNatural->Equals("water") : false;

		auto buildIter = relation->Tags.Find("building");
		auto partIter = relation->Tags.Find("building:part");

		//if this relation is building
		if	(	FoliageIterNatural && FoliageIterNatural->Equals("wood")
			||	FoliageIterLanduse && FoliageIterLanduse->Equals("forest")
			||	FoliageIterLeisure && (FoliageIterLeisure->Equals("park") || FoliageIterLeisure->Equals("garden"))
			||	partIter || buildIter || water
			)
		{
			FMultipolygonData polygon;

			UnclosedOuterContours.Empty();
			UnclosedInnerContours.Empty();

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

				bool isOuter = element.Value == "outer";
				auto& conts = isOuter ? polygon.Outer : polygon.Holes;
				auto& unclosedConts = isOuter ? UnclosedOuterContours : UnclosedInnerContours;

				if (contour.IsClosed())
				{
					contour.FixClockwise(isOuter);
					conts.Add(contour);
				}
				else
				{

					unclosedConts.Add(contour);
				}
			}

			polygon.Outer.Append(ULoaderHelper::FixRelationContours(UnclosedOuterContours));
			polygon.Holes.Append(ULoaderHelper::FixRelationContours(UnclosedInnerContours));

			polygon.Tags = relation->Tags;
			polygon.Origin = osmReader->GeoCoords;

			if (partIter || buildIter || water)
			{
				polygon.Tags.Add(TPair<FString, FString>("Type", "Exclude"));
			}
			else
			{
				if (FoliageIterLeisure)
				{
					polygon.Tags.Add(TPair<FString, FString>("typeRole", "park"));
				}
				else
				{
					polygon.Tags.Add(TPair<FString, FString>("typeRole", "forest"));
				}
				auto leafTag = relation->Tags.Find("leaf_type");
				if (leafTag)
				{
					polygon.Tags.Add(TPair<FString, FString>("leaf_type", *leafTag));
				}
				else
				{
					polygon.Tags.Add(TPair<FString, FString>("leaf_type", "mixed"));
				}
			}

			polygons.Add(polygon);
		}
	}
	return polygons;
}

#include "OSM/LoaderFoliageOsm.h"

#include "LoaderHelper.h"


void ULoaderFoliageOsm::SetOsmReader_Implementation(UOsmReader* inOsmReader)
{
	osmReader = inOsmReader;
}


TArray<FContourData> ULoaderFoliageOsm::GetFolliage_Implementation()
{
	TArray<FContourData> polygons;

	TArray<FContour> UnclosedOuterContours;
	TArray<FContour> UnclosedInnerContours;

	polygons.Empty();
	//find all building and building parts through ways
	for (auto wayP : osmReader->Ways)
	{
		auto way = wayP.second;
		auto FoliageIterNatural = way->Tags.Find("natural");
		auto FoliageIterLanduse = way->Tags.Find("landuse");
		auto FoliageIterLeisure = way->Tags.Find("leisure");

		auto buildIter = way->Tags.Find("building");
		auto partIter = way->Tags.Find("building:part");

		FContourData polygon;
		//if this is building or part
		if	(	FoliageIterNatural && FoliageIterNatural->Equals("wood")
			||	FoliageIterLanduse && FoliageIterLanduse->Equals("forest")
			||	FoliageIterLeisure && (FoliageIterLeisure->Equals("park") || FoliageIterLeisure->Equals("garden"))
			||	buildIter || partIter
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

			if (buildIter || partIter)
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
			}

			polygon.ZeroLat = osmReader->GeoCoords.ZeroLat;
			polygon.ZeroLon = osmReader->GeoCoords.ZeroLon;

			polygons.Add(polygon);
		}

		if (way->Tags.Contains("highway"))
		{
			FVector pointDelta;

			auto lanes = ULoaderHelper::TryGetTag(way->Tags, "lanes", ULoaderHelper::DEFAULT_LANES);
			auto width = ULoaderHelper::TryGetTag(way->Tags, "width", lanes * ULoaderHelper::DEFAULT_LANE_WIDTH);

			for (int i = 0; i < way->Nodes.size() - 1; i++)
			{
				FContourData roadPolygon;
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

				FVector radiusDeltas[capDensity + 1];

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
		auto relation = relationP.second;
		auto FoliageIterNatural = relation->Tags.Find("natural");
		auto FoliageIterLanduse = relation->Tags.Find("landuse");
		auto FoliageIterLeisure = relation->Tags.Find("leisure");

		auto partIter = relation->Tags.Find("building:part");

		//if this relation is building
		if	(	FoliageIterNatural && FoliageIterNatural->Equals("wood")
			||	FoliageIterLanduse && FoliageIterLanduse->Equals("forest")
			||	FoliageIterLeisure && (FoliageIterLeisure->Equals("park") || FoliageIterLeisure->Equals("garden"))
			||	partIter
			)
		{
			FContourData polygon;

			UnclosedOuterContours.Empty();
			UnclosedInnerContours.Empty();

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
					if (contour.IsClosed())
					{
						contour.FixClockwise();
						polygon.Outer.Add(contour);
					}
					else
					{
						UnclosedOuterContours.Add(contour);
					}
				}
				else if (element.second == "inner")
				{
					if (contour.IsClosed())
					{
						contour.FixClockwise(true);
						polygon.Holes.Add(contour);
					}
					else
					{
						UnclosedInnerContours.Add(contour);
					}
				}
			}

			polygon.Outer.Append(ULoaderHelper::FixRelationContours(UnclosedOuterContours));
			polygon.Holes.Append(ULoaderHelper::FixRelationContours(UnclosedInnerContours));

			polygon.Tags = relation->Tags;
			polygon.ZeroLat = osmReader->GeoCoords.ZeroLat;
			polygon.ZeroLon = osmReader->GeoCoords.ZeroLon;

			if (partIter)
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
			}

			polygons.Add(polygon);
		}
	}
	return polygons;
}

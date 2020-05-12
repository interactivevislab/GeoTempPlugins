#include "OSM/LoaderWaterOsm.h"

#include "LoaderHelper.h"


void ULoaderWaterOsm::SetOsmReader_Implementation(UOsmReader* inOsmReader)
{
	osmReader = inOsmReader;
}


TArray<FMultipolygonData> ULoaderWaterOsm::GetWater_Implementation()
{
	TArray<FMultipolygonData> polygons;

	TArray<FContour> UnclosedOuterContours;
	TArray<FContour> UnclosedInnerContours;

	polygons.Empty();
	ErrorRelations.Empty();
	DataParsedSuccessfully = true;
	//find all building and building parts through ways
	for (auto wayP : osmReader->Ways)
	{
		auto way = wayP.Value;
		auto FoliageIterNatural = way->Tags.Find("natural");

		FMultipolygonData polygon;
		//if this is building or part
		if (FoliageIterNatural && (FoliageIterNatural->Equals("water") || FoliageIterNatural->Equals("bay")))
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

		if (way->Tags.Contains("waterway"))
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
				polygon.Origin = osmReader->GeoCoords;

				polygons.Add(roadPolygon);
			}
		}
	}

	for (auto relationP : osmReader->Relations)
	{
		auto relation = relationP.Value;
		auto FoliageIterNatural = relation->Tags.Find("natural");

		//if this relation is building
		if (FoliageIterNatural && (FoliageIterNatural->Equals("water") || FoliageIterNatural->Equals("bay")))
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
			//bool clockwise;
			//if (relation->WayRoles.Num()==2)
			//{
			//	if (relation->Ways[0]->Nodes[0] == relation->Ways[1]->Nodes[0])
			//	{

			//	}
			//}
			bool goodRelation = true;
			polygon.Outer.Append(ULoaderHelper::FixAndCutRelationContours(UnclosedOuterContours, osmReader->BoundsRect, *relation, goodRelation, ErrorRelations));
			polygon.Holes.Append(ULoaderHelper::FixAndCutRelationContours(UnclosedInnerContours, osmReader->BoundsRect, *relation, goodRelation, ErrorRelations));
			DataParsedSuccessfully = DataParsedSuccessfully && goodRelation;
			if (!goodRelation)
			{
				continue;
			}
			polygon.Tags = relation->Tags;
			polygon.Origin = osmReader->GeoCoords;
			if (polygon.Outer.Num()==0)
			{
				if (polygon.Holes.Num() > 0)
				{
					FContour filler = FContour();
					filler.Points.Append(
						{
							FVector(osmReader->BoundsRect.X,osmReader->BoundsRect.Z,0),
							FVector(osmReader->BoundsRect.Y,osmReader->BoundsRect.Z,0),
							FVector(osmReader->BoundsRect.Y,osmReader->BoundsRect.W,0),
							FVector(osmReader->BoundsRect.X,osmReader->BoundsRect.W,0),
							FVector(osmReader->BoundsRect.X,osmReader->BoundsRect.Z,0),
						}
					);
					polygon.Outer.Add(filler);
				}
				else
				{
					continue;
				}
			}
			polygons.Add(polygon);
		}
	}
	return polygons;
}

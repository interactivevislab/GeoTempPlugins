#include "OSM/LoaderFoliageOsm.h"

#include "LoaderHelper.h"


inline const double signedArea(const TArray<FVector> &p)
{
	double A = 0;
	//========================================================//
	// Assumes:                                               //
	//    N+1 vertices:   p[0], p[1], ... , p[N-1], p[N]      //
	//    Closed polygon: p[0] = p[N]                         //
	// Returns:                                               //
	//    Signed area: +ve if anticlockwise, -ve if clockwise //
	//========================================================//
	int N = p.Num() - 1;
	for (int i = 0; i < N; i++) A += p[i].X * p[i + 1].Y - p[i + 1].X * p[i].Y;
	A *= 0.5;
	return A;
}


inline const TArray<FVector> border(const TArray<FVector> &p, double thickness)
{
	//=====================================================//
	// Assumes:                                            //
	//    N+1 vertices:   p[0], p[1], ... , p[N-1], p[N]   //
	//    Closed polygon: p[0] = p[N]                      //
	//    No zero-length sides                             //
	// Returns (by reference, as a parameter):             //
	//    Internal poly:  q[0], q[1], ... , q[N-1], q[N]   //
	//=====================================================//
	TArray<FVector> q;
	int N = p.Num() - 1;
	q.SetNum(N + 1);

	double x, y, X, Y, d, cross;

	double displacement = thickness;
	if (signedArea(p) < 0) displacement = -displacement;     // Detects clockwise order

	// Unit vector (x,y) along last edge
	x = p[N].X - p[N - 1].X;
	y = p[N].Y - p[N - 1].Y;
	d = sqrt(x * x + y * y);
	x /= d;
	y /= d;

	// Loop round the polygon, dealing with successive intersections of lines
	for (int i = 0; i < N; i++)
	{
		// Unit vector (X,Y) along previous edge
		X = x;
		Y = y;
		// Unit vector (x,y) along next edge
		x = p[i + 1].X - p[i].X;
		y = p[i + 1].Y - p[i].Y;
		d = sqrt(x * x + y * y);
		x /= d;
		y /= d;
		// New vertex
		cross = X * y - x * Y;
		if (abs(cross) < 0.0000000001f)      // Degenerate cases: 0 or 180 degrees at vertex
		{
			q[i].X = p[i].X - displacement * y;
			q[i].Y = p[i].Y + displacement * x;
		}
		else                             // Usual case
		{
			q[i].X = p[i].X + displacement * (x - X) / cross;
			q[i].Y = p[i].Y + displacement * (y - Y) / cross;
		}
	}

	// Close the inside polygon
	q[N] = q[0];

	return q;
}


TArray<FContour> FixRelationContours(TArray<FContour>& inUnclosedContours)
{
	TArray<FContour> closedContours = {};
	while (inUnclosedContours.Num() > 0)
	{
		if (closedContours.Num() == 0)
		{
			closedContours.Add(inUnclosedContours[0]);
			inUnclosedContours.RemoveAt(0);
			continue;
		}
		bool hasConnections = false;

		for (auto& contour : closedContours)
		{
			auto lastPointIndex = contour.Points.Num() - 1;
			int contourToRemove = -1;

			for (int i = 0; i < inUnclosedContours.Num(); i++)
			{
				if (contour.Points[lastPointIndex] == inUnclosedContours[i].Points[0])
				{
					for (int j = 1; j < inUnclosedContours[i].Points.Num(); j++)
					{
						contour.Points.Add(inUnclosedContours[i].Points[j]);
					}
					contourToRemove = i;
					break;
				}

				if (contour.Points[0] == inUnclosedContours[i].Points.Last())
				{
					for (int j = inUnclosedContours[i].Points.Num() - 2; j >= 0; j--)
					{
						contour.Points.Insert(inUnclosedContours[i].Points[j], 0);
					}
					contourToRemove = i;
					break;
				}

				if (contour.Points[0] == inUnclosedContours[i].Points[0])
				{
					for (int j = 1; j < inUnclosedContours[i].Points.Num(); j++)
					{
						contour.Points.Insert(inUnclosedContours[i].Points[j], 0);
					}
					contourToRemove = i;
					break;
				}

				if (contour.Points[lastPointIndex] == inUnclosedContours[i].Points.Last())
				{
					for (int j = inUnclosedContours[i].Points.Num() - 2; j >= 0; j--)
					{
						contour.Points.Add(inUnclosedContours[i].Points[j]);
					}
					contourToRemove = i;
					break;
				}
			}
			if (contourToRemove >= 0)
			{
				inUnclosedContours.RemoveAt(contourToRemove);
				hasConnections = true;
			}
			else
			{
				hasConnections = false;
			}
		}
		if (!hasConnections)
		{
			closedContours.Add(inUnclosedContours[0]);
			inUnclosedContours.RemoveAt(0);
			continue;
		}
	}
	for (auto& contour : closedContours)
	{
		if (!contour.IsClosed())
		{
			contour.FixClockwise();
		}
	}
	return closedContours;
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

			polygon.ZeroLat = osmReader->GeoCoords.ZeroLat;
			polygon.ZeroLon = osmReader->GeoCoords.ZeroLon;

			polygons.Add(polygon);
		}

		if (way->Tags.Contains("highway"))
		{
			TArray<FVector> roadPoints = {};
			TArray<FVector> inversePoints = {};
			FVector pointDelta;
			for (auto node : way->Nodes)
			{
				roadPoints.Add(node->Point);
				inversePoints.Insert(node->Point,0);
				if (inversePoints.Num()>1)
				{
					pointDelta = FVector::CrossProduct((inversePoints[1] - inversePoints[0]).GetSafeNormal(), FVector(0, 0, 0.5));
					inversePoints[1] = inversePoints[1] + pointDelta;
				}
			}
			inversePoints[0] = inversePoints[0] + pointDelta;
			roadPoints.Append(inversePoints);
			FVector firstPoint = roadPoints[0];
			roadPoints.Add(firstPoint);

			auto lanes = ULoaderHelper::TryGetTag(way->Tags, "lanes", ULoaderHelper::DEFAULT_LANES);
			auto width = ULoaderHelper::TryGetTag(way->Tags, "width", lanes * ULoaderHelper::DEFAULT_LANE_WIDTH);

			auto roadCont = FContour(border(roadPoints, width*50));
			polygon.Outer.Add(roadCont);

			polygon.Tags = way->Tags;
			polygon.Tags.Add(TPair<FString, FString>("Type", "Exclude"));

			polygons.Add(polygon);
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

			TArray<FContour> UnclosedOuterContours;
			TArray<FContour> UnclosedInnerContours;

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
			auto fixedContours = FixRelationContours(UnclosedOuterContours);
			
			for (auto& contour : fixedContours)
			{
					polygon.Outer.Add(contour);
			}

			polygon.Tags = relation->Tags;
			polygon.ZeroLat = osmReader->GeoCoords.ZeroLat;
			polygon.ZeroLon = osmReader->GeoCoords.ZeroLon;

			if (partIter)
			{
				polygon.Tags.Add(TPair<FString, FString>("Type", "Exclude"));
			}

			polygons.Add(polygon);
		}
	}
	return polygons;
}

#include "GeometryData.h"


void FMultipolygonData::Append(const FMultipolygonData& inOther)
{
	Outer.Append(inOther.Outer);
	Holes.Append(inOther.Holes);
}


FVector FMultipolygonData::BinaryParsePoint(uint8* inArray, int& outOffset,
	float inHeight)
{
	return BinaryParsePoint(inArray, outOffset, Origin, inHeight);
}


FVector FMultipolygonData::BinaryParsePoint(uint8* inArray, int& outOffset, FGeoCoords inGeoCoords, float inHeight)
{
	double* arr = reinterpret_cast<double*>(inArray + outOffset * sizeof(uint8));
	double lon = arr[0];
	double lat = arr[1];
	FVector coord = UGeoHelpers::GetLocalCoordinates(lon, lat, 0, inGeoCoords);
	FVector point = FVector(coord.X, coord.Y, inHeight);
	outOffset += 16;
	return point;
}


TArray<FVector> FMultipolygonData::BinaryParseCurve(uint8* inArray, int& outOffset,
	bool skipBOM, float inHeight)
{
	return BinaryParseCurve(inArray, outOffset, Origin, skipBOM, inHeight);
}


TArray<FVector> FMultipolygonData::BinaryParseCurve(uint8* inArray, int& outOffset, FGeoCoords inGeoCoords,
	bool skipBOM, float inHeight)
{
	if (skipBOM) {
		outOffset += 5;
	}

	uint32 count = *(reinterpret_cast<uint32*>(inArray + outOffset * sizeof(uint8)));
	outOffset += 4;

	TArray<FVector> points;
	for (uint32 i = 0; i < count; i++)
	{
		FVector point = BinaryParsePoint(inArray, outOffset, inGeoCoords, inHeight);
		points.Add(point);
	}
	return points;
}


FMultipolygonData FMultipolygonData::BinaryParsePolygon(uint8* inArray, int& outOffset,
	bool skipBOM, float inHeight)
{
	return BinaryParsePolygon(inArray, outOffset, Origin, skipBOM, inHeight);
}


FMultipolygonData FMultipolygonData::BinaryParsePolygon(uint8* inArray, int& outOffset, FGeoCoords inGeoCoords,
	bool skipBOM, float inHeight)
{
	if (skipBOM) {
		outOffset += 5;
	}

	uint32 count = *(reinterpret_cast<uint32*>(inArray + outOffset * sizeof(uint8)));
	outOffset += 4;

	FMultipolygonData poly;
	poly.Origin = inGeoCoords;	

	for (uint32 i = 0; i < count; i++)
	{
		auto points = BinaryParseCurve(inArray, outOffset, inGeoCoords, false, inHeight);
		if (i == 0) {
			poly.Outer.Add(points);
		}
		else
		{
			poly.Outer.Add(points);
		}
	}
	return poly;
}


FVector UGeometryHelpers::LinesIntersection(FVector inFirstLineStart, FVector inFirstLineEnd, FVector inSecondLineStart, FVector inSecondLineEnd)
{
	// Line AB represented as a1x + b1y = c1 
	double a1 = inFirstLineEnd.Y - inFirstLineStart.Y;
	double b1 = inFirstLineStart.X - inFirstLineEnd.X;
	double c1 = a1 * (inFirstLineStart.X) + b1 * (inFirstLineStart.Y);

	// Line CD represented as a2x + b2y = c2 
	double a2 = inSecondLineEnd.Y - inSecondLineStart.Y;
	double b2 = inSecondLineStart.X - inSecondLineEnd.X;
	double c2 = a2 * (inSecondLineStart.X) + b2 * (inSecondLineStart.Y);

	double determinant = a1 * b2 - a2 * b1;

	if (determinant == 0)
	{
		// The lines are parallel. This is simplified 
		// by returning a pair of FLT_MAX 
		return FVector(FLT_MAX, FLT_MAX, 0);
	}
	else
	{
		double x = (b2*c1 - b1 * c2) / determinant;
		double y = (a1*c2 - a2 * c1) / determinant;
		return FVector(x, y, 0);
	}
}


bool UGeometryHelpers::DoLineSegmentsIntersect(FVector inFirstLineStart, FVector inFirstLineEnd, FVector inSecondLineStart, FVector inSecondLineEnd, FVector& outIntersection)
{
	auto intersection = UGeometryHelpers::LinesIntersection(inFirstLineStart, inFirstLineEnd, inSecondLineStart, inSecondLineEnd);
	outIntersection = intersection;
	if (intersection.X == FLT_MAX &&
		intersection.Y == FLT_MAX)
	{
		return false;
	}
	auto minX = FMath::Min(inFirstLineStart.X, inFirstLineEnd.X);
	auto maxX = FMath::Max(inFirstLineStart.X, inFirstLineEnd.X);
	auto minY = FMath::Min(inFirstLineStart.Y, inFirstLineEnd.Y);
	auto maxY = FMath::Max(inFirstLineStart.Y, inFirstLineEnd.Y);
	if ((intersection.X >= minX && intersection.X <= maxX) && (intersection.Y >= minY && intersection.Y <= maxY))
	{
		return true;
	}
	return false;
}

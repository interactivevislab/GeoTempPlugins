#include "GeometryData.h"


void FContourData::Append(FContourData* inOther)
{
	Outer.Append(inOther->Outer);
	Holes.Append(inOther->Holes);
}


FVector* FContourData::BinaryParsePoint(uint8* inArray, int& outOffset, ProjectionType inProjection,
	float inHeight)
{
	return BinaryParsePoint(inArray, outOffset, FGeoCoords(inProjection, ZeroLon, ZeroLat), inHeight);
}


FVector* FContourData::BinaryParsePoint(uint8* inArray, int& outOffset, FGeoCoords inGeoCoords, float inHeight)
{
	double* arr = reinterpret_cast<double*>(inArray + outOffset * sizeof(uint8));
	double lon = arr[0];
	double lat = arr[1];
	FVector coord = UGeoHelpers::GetLocalCoordinates(lon, lat, 0, inGeoCoords);
	FVector* point = new FVector(coord.X, coord.Y, inHeight);
	outOffset += 16;
	return point;
}


TArray<FVector> FContourData::BinaryParseCurve(uint8* inArray, int& outOffset, ProjectionType inProjection,
	bool skipBOM, float inHeight)
{
	return BinaryParseCurve(inArray, outOffset, FGeoCoords(inProjection, ZeroLon, ZeroLat), skipBOM, inHeight);
}


TArray<FVector> FContourData::BinaryParseCurve(uint8* inArray, int& outOffset, FGeoCoords inGeoCoords,
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
		FVector* point = BinaryParsePoint(inArray, outOffset, inGeoCoords, inHeight);
		points.Add(*point);
	}
	return points;
}


FContourData* FContourData::BinaryParsePolygon(uint8* inArray, int& outOffset,
	ProjectionType inProjection, bool skipBOM, float inHeight)
{
	return BinaryParsePolygon(inArray, outOffset, FGeoCoords(inProjection, ZeroLon, ZeroLat), skipBOM, inHeight);
}


FContourData* FContourData::BinaryParsePolygon(uint8* inArray, int& outOffset, FGeoCoords inGeoCoords,
	bool skipBOM, float inHeight)
{
	if (skipBOM) {
		outOffset += 5;
	}

	uint32 count = *(reinterpret_cast<uint32*>(inArray + outOffset * sizeof(uint8)));
	outOffset += 4;

	FContourData* poly = new FContourData();
	poly->ZeroLat = inGeoCoords.ZeroLat;
	poly->ZeroLon = inGeoCoords.ZeroLon;

	for (uint32 i = 0; i < count; i++)
	{
		auto points = BinaryParseCurve(inArray, outOffset, inGeoCoords, false, inHeight);
		if (i == 0) {
			poly->Outer.Add(points);
		}
		else
		{
			poly->Outer.Add(points);
		}
	}
	return poly;
}
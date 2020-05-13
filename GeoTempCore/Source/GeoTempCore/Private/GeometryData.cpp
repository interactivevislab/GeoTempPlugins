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

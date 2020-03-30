// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <vector>
#include <string>

#include "CoreMinimal.h"
#include "Basics.Generated.h"

#ifndef PI
#define PI (3.1415926535897932)
#endif
#define DEG2RAD(a) ((a) / (180 / PI))
#define RAD2DEG(a) ((a) * (180 / PI))
#define EARTH_RADIUS 6378137
#define SCALE_MULT 100

struct FContour;

void GEOTEMPCORE_API  Triangulate(TArray<FContour>& outer, TArray<FContour>& inner, std::vector<FVector>& points, std::vector<int>& triangles, std::string flags, TArray<FContour> otherLines, int& contourPointsNum);

void GEOTEMPCORE_API  Triangulate(TArray<FContour>& outer, TArray<FContour>& inner, std::vector<FVector>& points, std::vector<int>& triangles, std::string flags = "");


UENUM(BlueprintType)
enum class ProjectionType : uint8
{
	WGS84_PsevdoMerkator	UMETA(DisplayName = "WGS84 Psevdo Merkator"),
	WGS84					UMETA(DisplayName = "WGS84"),
	LOCAL_METERS			UMETA(DisplayName = "Local meter coordinates"),
};

USTRUCT(BlueprintType)
struct GEOTEMPCORE_API FContour
{
	GENERATED_BODY()

		UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
		TArray<FVector> points;

	FContour()
	{
	} ;
	
	FContour(std::vector<FVector> initPoints) 
	{
		for (auto point : initPoints)
		{
			points.Add(point);
		}
	}

	FContour(TArray<FVector> initPoints)
	{
		for (auto point : initPoints)
		{
			points.Add(point);
		}
	}

	int LeftmostIndex()
	{
		int minInd = 0;
		float minX = points[0].X;
		for (int i = 1; i < points.Num(); i++)
		{
			if (points[i].X < minX)
			{
				minInd = i;
				minX = points[i].X;
			}
		}
		return minInd;
	}

	int RightmostIndex()
	{
		int minInd = 0;
		float maxX = points[0].X;
		for (int i = 1; i < points.Num(); i++)
		{
			if (points[i].X > maxX)
			{
				minInd = i;
				maxX = points[i].X;
			}
		}
		return minInd;
	}

	int TopmostIndex()
	{
		int minInd = 0;
		float minY = points[0].Y;
		for (int i = 1; i < points.Num(); i++)
		{
			if (points[i].Y < minY)
			{
				minInd = i;
				minY = points[i].Y;
			}
		}
		return minInd;
	}

	int BottommostIndex()
	{
		int minInd = 0;
		float maxY = points[0].Y;
		for (int i = 1; i < points.Num(); i++)
		{
			if (points[i].Y > maxY)
			{
				minInd = i;
				maxY = points[i].Y;
			}
		}
		return minInd;
	}

	//returns true if contour was reverted
	bool FixClockwise(bool reverse = false)
	{
		if ((points.Last() - points[0]).Size2D() > 1)
		{
			auto v = points[0];
			points.Add(v);
		}

		int i = LeftmostIndex();
		int i1 = (i + 1) % points.Num();
		if ((points[i1] - points[i]).Size2D() < 1) i1 = (i1 + 1) % points.Num();
		int i0 = (i - 1 + points.Num()) % points.Num();
		if ((points[i0] - points[i]).Size2D() < 1) i0 = (i0 - 1 + points.Num()) % points.Num();

		if ((FVector::CrossProduct(points[i] - points[i0], points[i1] - points[i]).Z * (reverse ? -1 : 1) < 0))
		{
			Algo::Reverse(points);
			return true;
		}
		return false;
	}

	void Cleanup()
	{
		for (int i = 0; i < points.Num();)
		{
			if ((points[i] - points[(i + 1) % points.Num()]).Size2D() < 5) points.RemoveAt(i);
			else { i++; };
		}		
	}
	
	static FContour RemoveCollinear(FContour contour, float threshold = 0.001f)
	{
		TArray<FVector> pointsNew;
		bool revert = contour.FixClockwise();
		int i0 = contour.LeftmostIndex();
		int i1 = (i0 + 1) % contour.points.Num();
		pointsNew.Add(contour.points[i0]);
		pointsNew.Add(contour.points[i1]);
		for (int i = (i1 + 1) % contour.points.Num(); i != i0; i = (i + 1) % contour.points.Num())
		{
			auto flag = true;
			while (flag && pointsNew.Num() > 1)
			{
				auto p0 = pointsNew[pointsNew.Num() - 2];
				auto p1 = pointsNew[pointsNew.Num() - 1];
				auto dir0 = p1 - p0;
				auto dir1 = contour.points[i] - p1;
				float turn = FMath::Abs(FVector::CrossProduct(dir0.GetSafeNormal(), dir1.GetSafeNormal()).Z);
				if (turn < threshold)
				{
					pointsNew.RemoveAt(pointsNew.Num() - 1);
				}
				else
				{
					flag = false;
				}
			}
			pointsNew.Add(contour.points[i]);
		}
		FContour cont;
		cont.points = pointsNew;
		if (revert) cont.FixClockwise(true);
		return cont;
	}

	static FContour MakeConvex(FContour contour)
	{
		TArray<FVector> pointsNew;
		bool revert = contour.FixClockwise();
		int i0 = contour.LeftmostIndex();

		int i1 = (i0 + 1) % contour.points.Num();
		pointsNew.Add(contour.points[i0]);
		pointsNew.Add(contour.points[i1]);
		for (int i = (i1 + 1) % contour.points.Num(); i != i0; i = (i + 1) % contour.points.Num())
		{
			auto flag = true;
			while (flag && pointsNew.Num() > 1)
			{
				auto p0 = pointsNew[pointsNew.Num() - 2];
				auto p1 = pointsNew[pointsNew.Num() - 1];
				auto dir0 = p1 - p0;
				auto dir1 = contour.points[i] - p1;

				if (FVector::CrossProduct(dir0, dir1).Z < 0)
				{
					pointsNew.RemoveAt(pointsNew.Num() - 1);
				}
				else
				{
					flag = false;
				}
			}
			pointsNew.Add(contour.points[i]);
		}
		FContour cont;
		cont.points = pointsNew;
		if (revert) cont.FixClockwise(true);
		return cont;
	}


	inline FContour RemoveCollinear(float treshold = 0.001f) const { return RemoveCollinear(*this, treshold); }
	inline FContour MakeConvex() const { return MakeConvex(*this); }
};



UCLASS()
class GEOTEMPCORE_API UGeoHelpers : public UObject
{
	GENERATED_BODY()

public:
	static FVector getLocalCoordinates(double x, double y, double z, ProjectionType projection, float originLon, float originLat)
	{
		switch (projection)
		{
		case ProjectionType::LOCAL_METERS: return FVector(x, y, z) * 100; break;
		case ProjectionType::WGS84_PsevdoMerkator: {
			double ox = DEG2RAD(originLon) * EARTH_RADIUS;
			double oy = log(tan(DEG2RAD(originLat) / 2 + PI / 4)) * EARTH_RADIUS;
			return (FVector(float((x - ox) * SCALE_MULT), float((y - oy) * SCALE_MULT), z * SCALE_MULT));
		}
												 break;
		case ProjectionType::WGS84: {
			double ox = DEG2RAD(originLon) * EARTH_RADIUS;
			double oy = log(tan(DEG2RAD(originLat) / 2 + PI / 4)) * EARTH_RADIUS;
			double s = cos(DEG2RAD(originLat));
			double x1 = DEG2RAD(x) * EARTH_RADIUS;
			double y1 = log(tan(DEG2RAD(y) / 2 + PI / 4)) * EARTH_RADIUS;

			return -FVector(float((ox + -x1) * SCALE_MULT * s), float((y1 - oy) * SCALE_MULT * s), z * SCALE_MULT);
		}
								  break;
		}

		return FVector::ZeroVector;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Default")
		static FVector2D convertToLonLat(float x, float y, ProjectionType projection, float originLon, float originLat)
	{
		switch (projection)
		{
		case ProjectionType::LOCAL_METERS:
			UE_LOG(LogTemp, Warning, TEXT("Attempt to get geocoordinates with local projection"));
			return FVector2D::ZeroVector; break;
		case ProjectionType::WGS84_PsevdoMerkator: {

			double ox = DEG2RAD(originLon) * EARTH_RADIUS;
			double oy = log(tan(DEG2RAD(originLat) / 2 + PI / 4)) * EARTH_RADIUS;

			double posX = ox + x / SCALE_MULT;
			double posY = oy + y / SCALE_MULT;

			return FVector2D(posX, posY);
		}
		case ProjectionType::WGS84: {
			double ox = DEG2RAD(originLon) * EARTH_RADIUS;
			double oy = log(tan(DEG2RAD(originLat) / 2 + PI / 4)) * EARTH_RADIUS;

			double s = cos(DEG2RAD(originLat));
			//double x1 = DEG2RAD(x) * EARTH_RADIUS;
			//double y1 = log(tan(DEG2RAD(y) / 2 + PI / 4)) * EARTH_RADIUS;
			double posX1 = x / SCALE_MULT / s + ox;
			double posY1 = -y / SCALE_MULT / s + oy;

			double posX = RAD2DEG(posX1 / EARTH_RADIUS);
			double posY = RAD2DEG(2 * atan(exp(posY1 / EARTH_RADIUS)) - PI / 2);
			return FVector2D(posX, posY);
		}
		}

		return FVector2D::ZeroVector;
	}
};

USTRUCT(BlueprintType)
struct GEOTEMPCORE_API  FPosgisLinesData
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
		TArray<FContour> lines;
};

USTRUCT(BlueprintType)
struct GEOTEMPCORE_API  FPosgisContourData
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
		TArray<FContour> Outer;

	//UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
		float ZeroLon;

	//UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
		float ZeroLat;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
		TArray<FContour> Holes;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
		TMap<FString, FString> Tags;

	void Append(FPosgisContourData* other)
	{
		Outer.Append(other->Outer);
		Holes.Append(other->Holes);
	}

public:

	inline FVector* BinaryParsePoint(uint8* array, int& offset, ProjectionType projection, float height = 0) {
		return BinaryParsePoint(array, offset, projection, ZeroLon, ZeroLat, height);
	}

	inline static FVector* BinaryParsePoint(uint8* array, int& offset, ProjectionType projection, float ZeroLon, float ZeroLat, float height = 0) {
		double* arr = ((double*)(array + offset * sizeof(uint8)));
		double lon = arr[0];
		double lat = arr[1];
		FVector coord = UGeoHelpers::getLocalCoordinates(lon, lat, 0, projection, ZeroLon, ZeroLat);
		FVector* point = new FVector(coord.X, coord.Y, height);
		offset += 16;
		return point;
	}

	inline TArray<FVector> BinaryParseCurve(uint8* array, int& offset, ProjectionType projection, bool skipBOM = false, float height = 0) {
		return BinaryParseCurve(array, offset, projection, ZeroLon, ZeroLat, skipBOM, height);
	}

	inline static TArray<FVector> BinaryParseCurve(uint8* array, int& offset, ProjectionType projection, float ZeroLon, float ZeroLat, bool skipBOM = false, float height = 0) {
		if (skipBOM) {
			offset += 5;
		}
		uint32 count = *((uint32*)(array + offset * sizeof(uint8)));
		offset += 4;
		TArray<FVector> points;
		for (uint32 i = 0; i < count; i++) {

			FVector* point = BinaryParsePoint(array, offset, projection, ZeroLon, ZeroLat, height);
			points.Add(*point);
		}
		return points;
	}

	inline FPosgisContourData* BinaryParsePolygon(uint8* array, int& offset, ProjectionType projection, bool skipBOM, float height = 0) {
		return BinaryParsePolygon(array, offset, projection, ZeroLon, ZeroLat, skipBOM, height);
	}

	inline static FPosgisContourData* BinaryParsePolygon(uint8* array, int& offset, ProjectionType projection, float ZeroLon, float ZeroLat, bool skipBOM, float height = 0) {
		if (skipBOM) {
			offset += 5;
		}
		uint32 count = *((uint32*)(array + offset * sizeof(uint8)));
		offset += 4;
		FPosgisContourData* poly = new FPosgisContourData();
		poly->ZeroLat = ZeroLat;
		poly->ZeroLon = ZeroLon;
		
		for (uint32 i = 0; i < count; i++) {
			auto points = BinaryParseCurve(array, offset, projection, ZeroLon, ZeroLat, false, height);
			if (i == 0) {
				poly->Outer.Add(points);
			}
			else {
				poly->Outer.Add(points);
			}
		}
		return poly;
	}
};

#pragma once

#include "CoreMinimal.h"

#include "Basics.h"

#include "GeometryData.generated.h"


USTRUCT(BlueprintType)
struct GEOTEMPCORE_API FWkbEntity
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
	TArray<uint8> Geometry;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
	TMap<FString, FString> Tags;
};


USTRUCT(BlueprintType)
struct GEOTEMPCORE_API FLinesData
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
	TArray<FContour> Lines;
};


USTRUCT(BlueprintType)
struct GEOTEMPCORE_API FContourData
{
	GENERATED_BODY()

public:

	float ZeroLon;
	float ZeroLat;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
	TArray<FContour> Outer;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
	TArray<FContour> Holes;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
	TMap<FString, FString> Tags;

	void Append(const FContourData& inOther);

	FVector BinaryParsePoint(uint8* inArray, int& outOffset, ProjectionType inProjection, float inHeight = 0);
	static FVector BinaryParsePoint(uint8* inArray, int& outOffset, FGeoCoords inGeoCoords, float inHeight = 0);

	TArray<FVector> BinaryParseCurve(uint8* inArray, int& outOffset, ProjectionType inProjection,
		bool inSkipBOM = false, float inHeight = 0);
	static TArray<FVector> BinaryParseCurve(uint8* inArray, int& outOffset, FGeoCoords inGeoCoords,
		bool inSkipBOM = false, float inHeight = 0);

	FContourData BinaryParsePolygon(uint8* inArray, int& outOffset, ProjectionType inProjection,
		bool inSkipBOM, float inHeight = 0);
	static FContourData BinaryParsePolygon(uint8* inArray, int& outOffset, FGeoCoords inGeoCoords,
		bool inSkipBOM, float inHeight = 0);
};

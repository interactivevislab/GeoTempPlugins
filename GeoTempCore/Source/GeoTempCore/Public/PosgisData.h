#pragma once

#include "CoreMinimal.h"

#include "Basics.h"

#include "PosgisData.Generated.h"


USTRUCT(BlueprintType)
struct GEOTEMPCORE_API FPosgisLinesData
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
	TArray<FContour> Lines;
};


USTRUCT(BlueprintType)
struct GEOTEMPCORE_API FPosgisContourData
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

	void Append(FPosgisContourData* inOther);

	inline FVector* BinaryParsePoint(uint8* inArray, int& outOffset, ProjectionType inProjection, float inHeight = 0);
	inline static FVector* BinaryParsePoint(uint8* inArray, int& outOffset, FGeoCoords inGeoCoords, float inHeight = 0);

	inline TArray<FVector> BinaryParseCurve(uint8* inArray, int& outOffset, ProjectionType inProjection,
		bool inSkipBOM = false, float inHeight = 0);
	inline static TArray<FVector> BinaryParseCurve(uint8* inArray, int& outOffset, FGeoCoords inGeoCoords,
		bool inSkipBOM = false, float inHeight = 0);

	inline FPosgisContourData* BinaryParsePolygon(uint8* inArray, int& outOffset, ProjectionType inProjection,
		bool inSkipBOM, float inHeight = 0);
	inline static FPosgisContourData* BinaryParsePolygon(uint8* inArray, int& outOffset, FGeoCoords inGeoCoords,
		bool inSkipBOM, float inHeight = 0);
};

#pragma once

#include "CoreMinimal.h"

#include "RoadsData.generated.h"


UENUM(BlueprintType)
enum class EHighwayType : uint8
{
	Auto	UMETA(DisplayName = "Auto"),
	Rail	UMETA(DisplayName = "Rail")
};


USTRUCT(BlueprintType)
struct GEOTEMPCORE_API FRoadSegment
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	EHighwayType Type;

	UPROPERTY(BlueprintReadOnly)
	int Lanes;

	UPROPERTY(BlueprintReadOnly)
	float Width;

	UPROPERTY(BlueprintReadOnly)
	int StartYear;

	UPROPERTY(BlueprintReadOnly)
	int EndYear;

	UPROPERTY(BlueprintReadOnly)
	int StartCrossroadId;

	UPROPERTY(BlueprintReadOnly)
	int EndCrossroadId;

	UPROPERTY(BlueprintReadOnly)
	TArray<FVector> AllPoints;

	UPROPERTY(BlueprintReadOnly)
	FString Change;
};


USTRUCT(BlueprintType)
struct GEOTEMPCORE_API FCrossroad
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FVector Location;

	UPROPERTY(BlueprintReadOnly)
	TMap<int, int> Roads;	//<RoadSegmentId, ÑrossroadId>
};


USTRUCT(BlueprintType)
struct GEOTEMPCORE_API FRoadNetwork
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TMap<int, FRoadSegment> Segments;

	UPROPERTY(BlueprintReadOnly)
	TMap<int, FCrossroad> Crossroads;
};
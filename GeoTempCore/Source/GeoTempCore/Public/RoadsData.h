#pragma once

#include "CoreMinimal.h"

#include "RoadsData.generated.h"


UENUM(BlueprintType)
enum class RoadType : uint8
{
	Rail	= 0 UMETA(DisplayName = "Rail"),
	Asphalt	= 1 UMETA(DisplayName = "Asphalt"),
	Dirt1	= 2 UMETA(DisplayName = "Dirt1"),
	Dirt2	= 3 UMETA(DisplayName = "Dirt2"),
	Brick	= 4 UMETA(DisplayName = "Brick"),
	Stone	= 5 UMETA(DisplayName = "Stone"),
	Sand	= 6 UMETA(DisplayName = "Sand")
};


UENUM(BlueprintType)
enum class EHighwayType : uint8
{
	Auto	UMETA(DisplayName = "Auto"),
	Rail	UMETA(DisplayName = "Rail")
};


#pragma region PostGisRoadsData

USTRUCT(BlueprintType)
struct GEOTEMPCORE_API FPostGisRoadLine
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FVector Start;

	UPROPERTY(BlueprintReadOnly)
	FVector End;

	UPROPERTY(BlueprintReadOnly)
	TArray<FVector> AllPoints;
};


USTRUCT(BlueprintType)
struct GEOTEMPCORE_API FPostGisRoadProperties
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	EHighwayType Highway;

	UPROPERTY(BlueprintReadOnly)
	float Angle;

	UPROPERTY(BlueprintReadOnly)
	float Length;

	UPROPERTY(BlueprintReadOnly)
	int ParentId;
};


USTRUCT(BlueprintType)
struct GEOTEMPCORE_API FPostGisRoadSegment
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FPostGisRoadLine Line;

	UPROPERTY(BlueprintReadOnly)
	EHighwayType Highway;

	UPROPERTY(BlueprintReadOnly)
	int Lanes;

	UPROPERTY(BlueprintReadOnly)
	float LaneWidth;

	UPROPERTY(BlueprintReadOnly)
	int YearStart;

	UPROPERTY(BlueprintReadOnly)
	int YearEnd;

	UPROPERTY(BlueprintReadOnly)
	FString Change;
};


USTRUCT(BlueprintType)
struct GEOTEMPCORE_API FPostGisRoadNetwork
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TMap<int, FPostGisRoadSegment> Segments;
};

#pragma endregion


#pragma region ProcessedRoadsData

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
	TMap<int, int> Roads;	//<RoadSegmentId, OtherÑrossroadId>
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

#pragma endregion

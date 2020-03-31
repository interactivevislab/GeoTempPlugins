#pragma once

#include "CoreMinimal.h"

#include "Road.generated.h"


enum class EHighwayType : uint8;
struct FApiRoadSegment;

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


USTRUCT(BlueprintType)
struct GEOTEMPROADS_API FApiRoadLine
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
struct GEOTEMPROADS_API	FApiRoadProperties
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
struct GEOTEMPROADS_API	FApiRoadSegment
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) 
	FApiRoadLine Line;
	
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
struct GEOTEMPROADS_API	FApiRoadNetwork
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) 
	TMap<int, FApiRoadSegment> Segments;
};

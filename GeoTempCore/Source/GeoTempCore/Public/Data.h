// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Basics.h"
#include "Data.generated.h"

USTRUCT(BlueprintType)
struct GEOTEMPCORE_API FBuildingDates
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		FDateTime BuildStart;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		FDateTime BuildEnd;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		FDateTime DemolishStart;
};
struct FBuilding;
struct GEOTEMPCORE_API FBuildingPart
{
	long Id;
	float floorHeight = 375.0f;
	int Floors;
	bool OverrideHeight;
	float Height;
	int MinFloors;
	float MinHeight;
	FString StylePalette;
	FBuilding* Owner;
	std::vector<FBuilding*> possibleOwners;
	FBuildingDates BuildingDates;
	FBuildingPart(const long id) { Id = id; Owner = NULL; }

	TArray<FContour> OuterConts;
	TArray<FContour> InnerConts;

	TArray<FContour> RoofData;
	
	void FixContours();
};

USTRUCT(BlueprintType)
struct GEOTEMPCORE_API FBuilding
{
	GENERATED_BODY()

	long Id = 0;
	std::string Type;	
	std::vector<FBuildingPart*> Parts;	
	TMap<FString, FString> Tags;

	FBuildingPart* MainPart;

	FBuilding() {};
	FBuilding(const long id) : Id(id) {};
};

UENUM(BlueprintType)
enum class EHighwayType : uint8
{
	Auto 	UMETA(DisplayName = "Auto"),
	Rail 	UMETA(DisplayName = "Rail")
};

USTRUCT(BlueprintType)
struct GEOTEMPCORE_API  FRoadSegment {

	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) EHighwayType Type;
	UPROPERTY(BlueprintReadOnly) int Lanes;
	UPROPERTY(BlueprintReadOnly) float Width;
	UPROPERTY(BlueprintReadOnly) int StartYear;
	UPROPERTY(BlueprintReadOnly) int EndYear;
	UPROPERTY(BlueprintReadOnly) int StartCrossroadId;
	UPROPERTY(BlueprintReadOnly) int EndCrossroadId;
	UPROPERTY(BlueprintReadOnly) TArray<FVector> allPoints;
	UPROPERTY(BlueprintReadOnly) FString Change;
};


USTRUCT(BlueprintType)
struct GEOTEMPCORE_API  FCrossroad {

	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) FVector Location;
	UPROPERTY(BlueprintReadOnly) TMap<int, int> Roads;	//RoadSegmentId, ÑrossroadId
};


USTRUCT(BlueprintType)
struct GEOTEMPCORE_API  FRoadNetwork {

	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) TMap<int, FRoadSegment> Segments;
	UPROPERTY(BlueprintReadOnly) TMap<int, FCrossroad> Crossroads;
};
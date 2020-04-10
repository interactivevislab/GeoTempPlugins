#pragma once

#include "CoreMinimal.h"

#include <vector>
#include <string>

#include "BuildingsData.generated.h"


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
struct FContour;


USTRUCT(BlueprintType)
struct GEOTEMPCORE_API FBuildingPart
{

	GENERATED_BODY()
	long Id;

	int Floors;
	int MinFloors;
	float FloorHeight = 375.0f;
	float Height;
	float MinHeight;
	
	bool OverrideHeight;
	
	FString StylePalette;

	FBuildingDates BuildingDates;

	TArray<FContour> OuterConts;
	TArray<FContour> InnerConts;
	TArray<FContour> RoofData;

	TMap<FString, FString> Tags;

	FLinearColor Color;

	FBuildingPart();
	
	FBuildingPart(const long inId);
};


USTRUCT(BlueprintType)
struct GEOTEMPCORE_API FBuilding
{
	GENERATED_BODY()

	long Id = 0;
	
	std::string Type;
	
	TMap<FString, FString> Tags;
	
	FBuildingPart MainPart;
	
	TArray<FBuildingPart> Parts;

	FString RoofType;

	FBuilding();
	FBuilding(const long inId);
};

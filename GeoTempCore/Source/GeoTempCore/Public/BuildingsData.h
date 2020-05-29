#pragma once

#include "CoreMinimal.h"

#include <vector>
#include <string>

#include "BuildingsData.generated.h"

/** Container for storing construction and demolition info for a building */
USTRUCT(BlueprintType)
struct GEOTEMPCORE_API FBuildingDates
{
	GENERATED_BODY()

	/** Date of beginning of a building construction */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FDateTime BuildStart;

	/** Date of end of a building construction */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FDateTime BuildEnd;

	/** Date of building demolition */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FDateTime DemolishStart;
};


struct FBuilding;
struct FContour;


/** Container for storing main parameters of a single part of the building*/
USTRUCT(BlueprintType)
struct GEOTEMPCORE_API FBuildingPart
{
	/** Id of this part*/
	GENERATED_BODY()
	long Id;

	/** Floors number of this part */
	int Floors;

	/** Floor on which this part begins (useful for hanged parts or roof extensions) */
	int MinFloors;

	/** Height of a single floor of a building */
	float FloorHeight = 375.0f;

	/** Height of this part */
	float Height;

	/** Elevation of bottom of this part (useful for hanged parts or roof extensions) */
	float MinHeight;

	/** Should this part use a height or floors number to determine its size in scene*/
	bool OverrideHeight;

	/** Name of style which can be used for further detailization */
	FString StylePalette;

	/** Dates of construction and demolition of this building */
	FBuildingDates BuildingDates;

	/** List of Outer contours of part footprint */
	TArray<FContour> OuterConts;

	/** List of inner contours (a.k.a. holes) of part footprint */
	TArray<FContour> InnerConts;

	/** Lines of roof apex*/
	TArray<FContour> RoofData;

	/** Dictionary of tags applied to this part on load. Often includes parameters related to its appearance and functionality */	
	TMap<FString, FString> Tags;

	/** Color of this building part */
	FLinearColor Color;

	/** Name of roof part style which can be used for further detailization */
	FString RoofType;
	
	FBuildingPart();
	
	FBuildingPart(const long inId);
};

/** Container for storing main parameters of a whole building*/
USTRUCT(BlueprintType)
struct GEOTEMPCORE_API FBuilding
{
	GENERATED_BODY()

	/** Id of this building */
	long Id = 0;

	/** type of this building, defined by building tag in osm */
	UPROPERTY(BlueprintReadOnly)
	FString Type;

	/** Dictionary of tags applied to the building on load. Can include address, material and other useful data*/
	UPROPERTY(BlueprintReadOnly)
	TMap<FString, FString> Tags;

	/** Main part of this building. This may be the real part of building, or contours describing its footprint */
	UPROPERTY(BlueprintReadOnly)
	FBuildingPart MainPart;

	/** List of all other parts of the building, except main */
	UPROPERTY(BlueprintReadOnly)
	TArray<FBuildingPart> Parts;

	/** List of entrances of the building*/
	UPROPERTY(BlueprintReadOnly)
	TArray<FVector> Entrances;

	/** A potential number of residents*/
	UPROPERTY(BlueprintReadOnly)
	int ResidentsCount;

	/** A size of a building area*/
	UPROPERTY(BlueprintReadOnly)
	float AreaSize;

	/** Name of roof part style which can be used for further detailization */
	UPROPERTY(BlueprintReadOnly)
	FString RoofType;

	FBuilding();
	FBuilding(const long inId);
};

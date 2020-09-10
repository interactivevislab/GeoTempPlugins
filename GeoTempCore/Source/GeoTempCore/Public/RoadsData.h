#pragma once

#include "CoreMinimal.h"

#include "RoadsData.generated.h"


/** Enum for type of road coating */
UENUM(BlueprintType)
enum class RoadType : uint8
{
	Rail	= 0 UMETA(DisplayName = "Rail"),	/**< Railroad coating. */ 
	Asphalt	= 1 UMETA(DisplayName = "Asphalt"),	/**< Asphalt coating. */ 
	Dirt1	= 2 UMETA(DisplayName = "Dirt1"),	/**< First dirt type coating. */ 
	Dirt2	= 3 UMETA(DisplayName = "Dirt2"),	/**< Second dirt type coating. */ 
	Brick	= 4 UMETA(DisplayName = "Brick"),	/**< Brick coating. */ 
	Stone	= 5 UMETA(DisplayName = "Stone"),	/**< Stone coating. */ 
	Sand	= 6 UMETA(DisplayName = "Sand")		/**< Sand coating. */ 
};


/** Enum for roads' types, in terms of transport. */
UENUM(BlueprintType)
enum class EHighwayType : uint8
{
	Auto	UMETA(DisplayName = "Auto"),	/**< Highway. */ 
	Rail	UMETA(DisplayName = "Rail"),	/**< Railroad. */ 
	Footway	UMETA(DisplayName = "Footway")	/**< Footway. */
};


#pragma region PostGisRoadsData


/** Struct for description the shape of the road in PostGis. */
USTRUCT(BlueprintType)
struct GEOTEMPCORE_API FPostGisRoadLine
{
	GENERATED_BODY()

	/** First point in road line. */
	UPROPERTY(BlueprintReadOnly)
	FVector Start;

	/** Last point in road line. */
	UPROPERTY(BlueprintReadOnly)
	FVector End;

	/** Array of all road line's points. */
	UPROPERTY(BlueprintReadOnly)
	TArray<FVector> AllPoints;
};


/** Struct for description the segment of the road in PostGis. */
USTRUCT(BlueprintType)
struct GEOTEMPCORE_API FPostGisRoadSegment
{
	GENERATED_BODY()

	/** Shape of road segment. */
	UPROPERTY(BlueprintReadOnly)
	FPostGisRoadLine Line;

	/** Type of road. */
	UPROPERTY(BlueprintReadOnly)
	EHighwayType Highway;

	/** Number of lanes. */
	UPROPERTY(BlueprintReadOnly)
	int Lanes;

	/** Width of one lane. */
	UPROPERTY(BlueprintReadOnly)
	float LaneWidth;
};


/** Struct for description the road network in PostGis. */
USTRUCT(BlueprintType)
struct GEOTEMPCORE_API FPostGisRoadNetwork
{
	GENERATED_BODY()

	/** All road segments by IDs. */
	UPROPERTY(BlueprintReadOnly)
	TMap<int, FPostGisRoadSegment> Segments;
};

#pragma endregion


#pragma region OsmRoadsData

/** Struct for description the segment of the road in OSM. */
USTRUCT(BlueprintType)
struct GEOTEMPCORE_API FOsmRoadSegment
{
	GENERATED_BODY()

	/** Array of road segment points. */
	UPROPERTY(BlueprintReadOnly)
	TArray<FVector> Points;

	/** Names and values of road segment tags. */
	UPROPERTY(BlueprintReadOnly)
	TMap<FString, FString> Tags;
};


/** Struct for description the road network of the road in OSM. */
USTRUCT(BlueprintType)
struct GEOTEMPCORE_API FOsmRoadNetwork
{
	GENERATED_BODY()

	/** All road segments by IDs. */
	UPROPERTY(BlueprintReadOnly)
	TMap<int, FOsmRoadSegment> Segments;
};

#pragma endregion


#pragma region ProcessedRoadsData

/** Struct for description the segment of the road. */
USTRUCT(BlueprintType)
struct GEOTEMPCORE_API FRoadSegment
{
	GENERATED_BODY()

	/** Type of road. */
	UPROPERTY(BlueprintReadOnly)
	EHighwayType Type;

	/** Whether the road is oneway only. */
	UPROPERTY(BlueprintReadOnly)
	bool Oneway = false;

	/** Number of lanes. */
	UPROPERTY(BlueprintReadOnly)
	int Lanes;

	/** Number of forward lanes. */
	UPROPERTY(BlueprintReadOnly)
	int LanesForward;

	/** Number of backward lanes. */
	UPROPERTY(BlueprintReadOnly)
	int LanesBackward;

	/** Width of one lane. */
	UPROPERTY(BlueprintReadOnly)
	float Width;

	/** ID of first crossroad at the end of road segment. */
	UPROPERTY(BlueprintReadOnly)
	int StartCrossroadId;

	/** ID of second crossroad at the end of road segment. */
	UPROPERTY(BlueprintReadOnly)
	int EndCrossroadId;

	/** Array of all road segment's points. */
	UPROPERTY(BlueprintReadOnly)
	TArray<FVector> AllPoints;
};


/** Struct for description the crossroad. */
USTRUCT(BlueprintType)
struct GEOTEMPCORE_API FCrossroad
{
	GENERATED_BODY()

	/** Point where the crossroad is located */
	UPROPERTY(BlueprintReadOnly)
	FVector Location;

	/** All road segments connected to the crossroad in the format "Road segment's ID - another crossroad's ID" */
	UPROPERTY(BlueprintReadOnly)
	TMap<int, int> Roads;
};


/** Struct for description the road network. */
USTRUCT(BlueprintType)
struct GEOTEMPCORE_API FRoadNetwork
{
	GENERATED_BODY()

	/** All road segments by IDs. */
	UPROPERTY(BlueprintReadOnly)
	TMap<int, FRoadSegment> Segments;

	/** All crossroads by IDs. */
	UPROPERTY(BlueprintReadOnly)
	TMap<int, FCrossroad> Crossroads;

	/** All traffic lights by IDs. */
	UPROPERTY(BlueprintReadOnly)
	TMap<int, FVector> TrafficLights;

	/** Road entries into an area. */
	UPROPERTY(BlueprintReadOnly)
	TArray<FVector> EntryPoints;
};

#pragma endregion

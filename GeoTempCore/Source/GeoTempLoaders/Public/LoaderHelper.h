#pragma once

#include "CoreMinimal.h"

#include "RoadsData.h"
#include "GeometryData.h"
#include "OsmData.h"

#include "LoaderHelper.Generated.h"


/** Static class with utility functions for data loaders. */
UCLASS()
class GEOTEMPLOADERS_API ULoaderHelper : public UObject
{
	GENERATED_BODY()

public:

	/** Default value for number lines in highway. */
	const static int DEFAULT_LANES;

	/** Default value line width in highway. */
	const static float DEFAULT_LANE_WIDTH;

	/** Creates a complete road network structure based on data from road segments. */
	static FRoadNetwork ConstructRoadNetwork(const TArray<FRoadSegment>& inRoadSegments);

	/** Combines polygons from multiple lines. */
	static TArray<FContour> FixRelationContours(TArray<FContour>& inUnclosedContours, int inRelationId, bool& outGoodData, TSet<int>& outErrorRelations);

	/** Combines and cuts polygons from multiple lines. */
	static TArray<FContour> FixAndCutRelationContours(TArray<FContour>& inUnclosedContour, FVector4 inBounds, int inRelationId, bool& outGoodData, TSet<int>& outErrorRelations);

	/** Cut multiple polygons to fit inside specific bounds. */
	static TArray<FContour> CutPolygonsByBounds(TArray<FContour>& inContour, FVector4 inBounds);

	/** Cut single polygon to fit inside specific bounds. */
	static TArray<FContour> CutPolygonByBounds(FContour inContour, FVector4 inBounds);

	/** Cut contour to fit inside specific bounds. */
	static TArray<FContour> CutContourByBounds(FContour inContour, FVector4 inBounds, TArray<FVector>& outIntersectionPoints);

	/** Gets point of intersection of a segment with square bounds. */
	static FVector GetSegmentIntersectionWithBounds(FVector inOuterPoint, FVector inInnerPoint, FVector4 inBounds);

#pragma region Tags

	/** Casts FString to another type. */
	template<typename ValueType>
	static ValueType ValueFromString(FString inString)
	{
		return ValueType(inString);
	}


	/** Casts FString to FString (returns the same value). */
	template<>
	static FString ValueFromString<FString>(FString inString)
	{
		return inString;
	}


	/** Casts FString to int. */
	template<>
	static int ValueFromString<int>(FString inString)
	{
		return FCString::Atoi(*inString);
	}


	/** Casts FString to float. */
	template<>
	static float ValueFromString<float>(FString inString)
	{
		return FCString::Atof(*inString);
	}


	/**
	* \fn TryGetTag
	* \brief Tries to read tag value from TMap.
	*
	* @param inTags		TMap of tags' names and values.
	* @param inTag		Tag to find.
	* @param inAltValue	Value to return if tag not found.
	*/
	template<typename ValueType>
	static ValueType TryGetTag(TMap<FString, FString> inTags, FString inTag, ValueType inAltValue)
	{
		auto value = inTags.Find(inTag);
		if (value != nullptr)
		{
			return ValueFromString<ValueType>(*value);
		}
		else
		{
			return inAltValue;
		}
	}

#pragma endregion

};

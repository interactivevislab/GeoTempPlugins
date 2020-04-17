#pragma once

#include "CoreMinimal.h"

#include "RoadsData.h"
#include "GeometryData.h"

#include "LoaderHelper.Generated.h"


UCLASS()
class GEOTEMPLOADERS_API ULoaderHelper : public UObject
{
	GENERATED_BODY()

public:

	const static int DEFAULT_LANES;
	const static float DEFAULT_LANE_WIDTH;

	static FRoadNetwork ConstructRoadNetwork(TArray<FRoadSegment> inRoadSegments);

	static TArray<FContour> FixRelationContours(TArray<FContour>& inUnclosedContours);

#pragma region Tags

	template<typename ValueType>
	static ValueType ValueFromString(FString inString)
	{
		return ValueType(inString);
	}


	template<>
	static FString ValueFromString<FString>(FString inString)
	{
		return inString;
	}


	template<>
	static int ValueFromString<int>(FString inString)
	{
		return FCString::Atoi(*inString);
	}


	template<>
	static float ValueFromString<float>(FString inString)
	{
		return FCString::Atof(*inString);
	}


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

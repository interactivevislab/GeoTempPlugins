#include "CoreMinimal.h"

#include "RoadsData.h"

#include "LoaderHelper.Generated.h"


UCLASS()
class GEOTEMPCORE_API ULoaderHelper : public UObject
{
	GENERATED_BODY()

public:

	static FRoadNetwork ConstructRoadNetwork(TArray<FRoadSegment> inRoadSegments);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	static FRoadNetwork GetRoadNetworkForYear(FRoadNetwork inFullRoadNetwork, int inYear);

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

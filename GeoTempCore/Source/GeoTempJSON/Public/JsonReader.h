#pragma once

#include "CoreMinimal.h"

#include "GameFramework/Actor.h"
#include "Dom/JsonObject.h"

#include "GeometryData.h"

#include "JsonReader.generated.h"


UENUM(BlueprintType)
enum class EGeometryType : uint8
{
	Point				UMETA(DisplayName = "Point"),
	LineString			UMETA(DisplayName = "LineString"),
	Polygon				UMETA(DisplayName = "Polygon"),
	MultiPoint			UMETA(DisplayName = "MultiPoint"),
	MultiLineString		UMETA(DisplayName = "MultiLineString"),
	MultiPolygon		UMETA(DisplayName = "MultiPolygon")
};


UCLASS()
class GEOTEMPJSON_API AJsonReader : public AActor
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TMap<FString, FString> JSONTags;

	UPROPERTY(BlueprintReadWrite)
	FString JsonString;

	FString TabString;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FGeoCoords GeoCoords;

	TArray<FContourData> GlobalContoursWithData;

	typedef TSharedPtr<FJsonValue> JsonValuesPtr;

	AJsonReader();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	TArray<FContourData> ReadContoursFromFile(FString inFilepath, FGeoCoords inGeoCoords);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	TArray<FContourData> ReadContoursFromString(FString inJsonString, FGeoCoords inGeoCoords);

	TArray<FContourData> ReadContoursFromJSON(TSharedPtr<FJsonObject> inJsonObject, FGeoCoords inGeoCoords);

	UFUNCTION(BlueprintCallable, CallInEditor)
	void ParseJSON();

	void ParseRecursion(TMap<FString, JsonValuesPtr> inValues);
	void ParseArray(TArray<JsonValuesPtr> inValues);

	void ParseFeatures(TArray<JsonValuesPtr> inFeatureArray, TArray<FContourData>& outContoursWithData);

	void ParsePolygon(TArray<JsonValuesPtr> inGeometry, FContourData& outContourData);
	void ParseMultiPolygon(TArray<JsonValuesPtr> inGeometry, FContourData& outContourData);

	template <typename EnumType>
	static FORCEINLINE EnumType GetEnumValueFromString(const FString& inEnumName, const FString& inString)
	{
		UEnum* Enum = FindObject<UEnum>(ANY_PACKAGE, *inEnumName, true);
		if (!Enum)
		{
			return EnumType(0);
		}
		return (EnumType)Enum->GetIndexByName(FName(*inString));
	}
};

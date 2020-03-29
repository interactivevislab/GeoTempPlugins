#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Basics.h"
#include "Dom/JsonObject.h"
#include "GeoJSONLoader.generated.h"



UENUM(BlueprintType)
enum class EGeometryType : uint8
{
	Point 		UMETA(DisplayName = "Point"),
	LineString 	UMETA(DisplayName = "LineString"),
	Polygon		UMETA(DisplayName = "Polygon"),
	MultiPoint 		UMETA(DisplayName = "MultiPoint"),
	MultiLineString 	UMETA(DisplayName = "MultiLineString"),
	MultiPolygon		UMETA(DisplayName = "MultiPolygon")
};

UCLASS()
class GEOTEMPJSON_API   AJsonLoader : public AActor
{
	GENERATED_BODY()

public:
	AJsonLoader();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Default")
		TArray<FPosgisContourData> ReadContoursFromFile(FString filepath, ProjectionType projection, float originLon, float originLat);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Default")
	TArray<FPosgisContourData> ReadContoursFromString(FString JsonString, ProjectionType projection, float originLon, float originLat);

	TArray<FPosgisContourData> ReadContoursFromJSON(TSharedPtr<FJsonObject> JsonObject, ProjectionType projection, float originLon, float originLat);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="default")
		TMap<FString, FString> JSONTags;

	UPROPERTY(BlueprintReadWrite, Category="default")
		FString JSONString;

	FString TabString;

	TArray<FPosgisContourData> GlobalContoursWithData;
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="default")
	ProjectionType _projection;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="default")
	float _originLon;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="default")
	float _originLat;

public:

	UFUNCTION(BlueprintCallable, CallInEditor, Category="Default")
		void ParseJSON();

	void ParseRecursion(TMap<FString, TSharedPtr<FJsonValue>> Values);
	void ParseArray(TArray<TSharedPtr<FJsonValue>> Values);

	void ParseFeatures(TArray<TSharedPtr<FJsonValue>> FeatureArray, TArray<FPosgisContourData>& ContoursWithData);

	void ParsePolygon(TArray<TSharedPtr<FJsonValue>> geometry, FPosgisContourData& contourData);
	void ParseMultiPolygon(TArray<TSharedPtr<FJsonValue>> geometry, FPosgisContourData& contourData);

	template <typename EnumType>
	static FORCEINLINE EnumType GetEnumValueFromString(const FString& EnumName, const FString& String)
	{
		UEnum* Enum = FindObject<UEnum>(ANY_PACKAGE, *EnumName, true);
		if (!Enum)
		{
			return EnumType(0);
		}
		return (EnumType)Enum->GetIndexByName(FName(*String));
	}
};

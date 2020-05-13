#pragma once

#include "CoreMinimal.h"

#include "Dom/JsonObject.h"

#include "GeometryData.h"

#include "JsonReader.generated.h"


/**
* \class UJsonReader
* \brief Class for reading contours data from JSON.
*
* @see FContourData
*/
UCLASS()
class GEOTEMPJSON_API UJsonReader : public UObject
{
    GENERATED_BODY()

public:

    /** Read contours data from JSON file, converting coordinates to local. */
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    TMap<FString, FString> JSONTags;

    
    UPROPERTY(BlueprintReadWrite)
    FString JsonString;

    FString TabString;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FGeoCoords GeoCoords;

    TArray<FMultipolygonData> GlobalContoursWithData;

    typedef TSharedPtr<FJsonValue> JsonValuesPtr;

    /** Read contours data from JSON file, converting coordinates to local. */
    UFUNCTION(BlueprintCallable, BlueprintPure)
    TArray<FMultipolygonData> ReadContoursFromFile(FString inFilepath, FGeoCoords inGeoCoords);

    /** Read contours data from JSON string, converting coordinates to local. */
    UFUNCTION(BlueprintCallable, BlueprintPure)
    TArray<FMultipolygonData> ReadContoursFromString(FString inJsonString, FGeoCoords inGeoCoords);

    /** Read contours data from JSON object, converting coordinates to local. */
    TArray<FMultipolygonData> ReadContoursFromJSON(TSharedPtr<FJsonObject> inJsonObject, FGeoCoords inGeoCoords);
        

private:

    /** Buffer for JSON string. */
    FString jsonString;

    /** Buffer for tabulation string. */
    FString tabString;

    /** Buffer for coordiantes. */
    FGeoCoords geoCoords;

    typedef TSharedPtr<FJsonValue> JsonValuesPtr;

    /** Parses recursion. */
    void ParseRecursion(TMap<FString, JsonValuesPtr> inValues);

    /** Parses data array. */
    void ParseArray(TArray<JsonValuesPtr> inValues);

    void ParseFeatures(TArray<JsonValuesPtr> inFeatureArray, TArray<FMultipolygonData>& outContoursWithData);
    /** Parses features into countour data. */

    /** Parses polygon into countour data. */
    void ParsePolygon(TArray<JsonValuesPtr> inGeometry, FMultipolygonData& outContourData);

    /** Parses multipolygon into countour data. */
    void ParseMultiPolygon(TArray<JsonValuesPtr> inGeometry, FMultipolygonData& outContourData);

    /** Converts string to enum. */
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

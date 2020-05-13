#include "JsonReader.h"


enum class EGeometryType : uint8
{
    Point                UMETA(DisplayName = "Point"),
    LineString            UMETA(DisplayName = "LineString"),
    Polygon                UMETA(DisplayName = "Polygon"),
    MultiPoint            UMETA(DisplayName = "MultiPoint"),
    MultiLineString        UMETA(DisplayName = "MultiLineString"),
    MultiPolygon        UMETA(DisplayName = "MultiPolygon")
};


TArray<FMultipolygonData> UJsonReader::ReadContoursFromFile(FString inFilepath, FGeoCoords inGeoCoords)
{
    const FString jsonFilePath = inFilepath;
    FString bufferString;
    FFileHelper::LoadFileToString(bufferString, *jsonFilePath);

    return ReadContoursFromString(bufferString, inGeoCoords);
}


TArray<FMultipolygonData> UJsonReader::ReadContoursFromString(FString inJsonString, FGeoCoords inGeoCoords)
{
    TSharedPtr<FJsonObject>        jsonObject = MakeShareable(new FJsonObject());
    TSharedRef<TJsonReader<>>    jsonReader = TJsonReaderFactory<>::Create(inJsonString);

    TArray<FMultipolygonData> contoursWithData;
    if (FJsonSerializer::Deserialize(jsonReader, jsonObject))
    {
        contoursWithData = ReadContoursFromJSON(jsonObject, inGeoCoords);
    }

    return contoursWithData;
}


TArray<FMultipolygonData> UJsonReader::ReadContoursFromJSON(TSharedPtr<FJsonObject> inJsonObject, FGeoCoords inGeoCoords)
{
    geoCoords = inGeoCoords;

    TArray<FMultipolygonData> contoursWithData;
    auto featureArray = inJsonObject->GetArrayField("features");
    ParseFeatures(featureArray, contoursWithData);
    return contoursWithData;
}


void UJsonReader::ParseFeatures(TArray<JsonValuesPtr> inFeatureArray, TArray<FMultipolygonData>& outContoursWithData)
{
    for (auto feature : inFeatureArray)
    {
        auto properties = feature->AsObject()->GetObjectField("properties");
        TMap<FString, FString> featureTags;
        for (auto value : properties->Values)
        {
            featureTags.Add(value.Key, value.Value->AsString());
        }
        FMultipolygonData contourData = FMultipolygonData();
        contourData.Tags = featureTags;

        auto    geometry = feature->AsObject()->GetObjectField("geometry");
        FString geomType = geometry->GetStringField("type");

        if (geomType != "MultiPolygon" && geomType != "Polygon")
        {
            return;
        }

        EGeometryType geometryType = GetEnumValueFromString<EGeometryType>("EGeometryType", geomType);  //Back From String!

        auto coords = geometry->GetArrayField("coordinates");

        switch (geometryType)
        {
            case EGeometryType::Polygon:
            {
                ParsePolygon(coords, contourData);
                break;
            }
            case EGeometryType::MultiPolygon:
            {
                ParseMultiPolygon(coords, contourData);
                break;
            }
        }

        outContoursWithData.Add(contourData);
    }
}


void UJsonReader::ParsePolygon(TArray<JsonValuesPtr> inGeometry, FMultipolygonData& outContourData)
{
    FContour contour;
    auto coords = inGeometry[0]->AsArray();
    TArray<FVector> points;

    for (int i = 0; i < coords.Num(); i++)
    {
        auto pointCoords = coords[i]->AsArray();
        points.Add(UGeoHelpers::GetLocalCoordinates(pointCoords[0]->AsNumber(), pointCoords[1]->AsNumber(), 0, geoCoords));
    }
    contour.Points = points;

    outContourData.Outer.Add(contour);

    contour = FContour();
    for (int i = 1; i < inGeometry.Num(); i++)
    {
        coords = inGeometry[i]->AsArray();

        TArray<FVector> holesPoints;
        for (int j = 0; j < coords.Num(); j++)
        {
            auto pointCoords = coords[j]->AsArray();
            holesPoints.Add(UGeoHelpers::GetLocalCoordinates(pointCoords[0]->AsNumber(), pointCoords[1]->AsNumber(), 0, geoCoords));
        }
        contour.Points = holesPoints;
    }

    if (contour.Points.Num()>0)
    {
        outContourData.Holes.Add(contour);
    }
}


void UJsonReader::ParseMultiPolygon(TArray<JsonValuesPtr> inGeometry, FMultipolygonData& inContourData)
{
    for (auto geom : inGeometry)
    {
        ParsePolygon(geom->AsArray(), inContourData);
    }
}


void UJsonReader::ParseRecursion(TMap<FString, JsonValuesPtr> inValues)
{
    for (auto val : inValues)
    {
        jsonString += tabString + "\"" + val.Key + "\":";

        switch (val.Value->Type)
        {
            case EJson::Object:
                jsonString    += "\n" + tabString + "{\n";
                tabString    += "\t";
                ParseRecursion(val.Value->AsObject()->Values);
                tabString.RemoveFromEnd("\t");
                jsonString    += "\n" + tabString + "}";
                break;

            case EJson::Array:
                jsonString    += "\n" + tabString + "[\n";
                tabString    += "\t";
                ParseArray(val.Value->AsArray());
                tabString.RemoveFromEnd("\t");
                jsonString    += "\n" + tabString + "]";
                break;

            default:
                jsonString    += " \"" + val.Value->AsString() + "\"";
        }

        jsonString += ",\n";
    }
    jsonString.RemoveFromEnd(",\n");
}


void UJsonReader::ParseArray(TArray<JsonValuesPtr> inValues)
{
    for (auto val : inValues)
    {
        switch (val->Type)
        {
            case EJson::Object:
                jsonString    += "\n" + tabString + "{\n";
                tabString    += "\t";
                ParseRecursion(val->AsObject()->Values);
                tabString.RemoveFromEnd("\t");
                jsonString    += "\n" + tabString + "}";
                break;

            case EJson::Array:
                jsonString    += "\n" + tabString + "[\n" + tabString;
                tabString    += "\t";
                ParseArray(val->AsArray());
                tabString.RemoveFromEnd("\t");
                jsonString    += "\n" + tabString + "]";
                break;

            default:
                jsonString    += " \"" + val->AsString() + "\"";
        }

        jsonString += ", ";
    }
    jsonString.RemoveFromEnd(", ");
}

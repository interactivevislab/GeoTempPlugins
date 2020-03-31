#include "GeoJSONLoader.h"


AJsonLoader::AJsonLoader()
{
	PrimaryActorTick.bCanEverTick = true;
}


TArray<FPosgisContourData> AJsonLoader::ReadContoursFromFile(FString filepath,
	ProjectionType projection, float originLon, float originLat)
{
	const FString JsonFilePath = filepath;
	FString JsonString;
	FFileHelper::LoadFileToString(JsonString, *JsonFilePath);

	return ReadContoursFromString(JsonString, projection, originLon, originLat);
}


TArray<FPosgisContourData> AJsonLoader::ReadContoursFromString(FString JsonString, 
	ProjectionType projection, float originLon, float originLat) 
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(JsonString);

	TArray<FPosgisContourData> ContoursWithData;
	if (FJsonSerializer::Deserialize(JsonReader, JsonObject))
	{
		ContoursWithData = ReadContoursFromJSON(JsonObject, projection, originLon, originLat);
	}

	return ContoursWithData;
}


TArray<FPosgisContourData> AJsonLoader::ReadContoursFromJSON(TSharedPtr<FJsonObject> JsonObject, ProjectionType projection, float originLon, float originLat) {
	_projection = projection;
	_originLon = originLon;
	_originLat = originLat;

	TArray<FPosgisContourData> ContoursWithData;
	auto FeatureArray = JsonObject->GetArrayField("features");
	ParseFeatures(FeatureArray, ContoursWithData);
	return ContoursWithData;
}

void AJsonLoader::ParseJSON()
{
	const FString JsonFilePath = FPaths::ProjectContentDir() + "Buildings Layer.geojson";
	FString JsonString;

	FFileHelper::LoadFileToString(JsonString, *JsonFilePath);

	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(JsonString);

	if (FJsonSerializer::Deserialize(JsonReader, JsonObject))
	{
		TArray<TSharedPtr<FJsonValue>> FeatureArray = JsonObject->GetArrayField("features");
		FString ExampleString = JsonObject->GetStringField("exampleString");
		JSONString += "{\n";
		TabString += "\t";
		ParseRecursion(JsonObject->Values);
		JSONString += "}";
	}
}

void AJsonLoader::ParseFeatures(TArray<TSharedPtr<FJsonValue>> FeatureArray, TArray<FPosgisContourData>& ContoursWithData)
{
	for (auto feature : FeatureArray)
	{
		auto properties = feature->AsObject()->GetObjectField("properties");
		TMap<FString, FString> FeatureTags;
		for (auto value : properties->Values)
		{
			FeatureTags.Add(value.Key, value.Value->AsString());
		}
		FPosgisContourData contourData = FPosgisContourData();
		contourData.Tags = FeatureTags;


		auto geometry = feature->AsObject()->GetObjectField("geometry");
		FString geomtype = geometry->GetStringField("type");

		if (geomtype != "MultiPolygon" && geomtype != "Polygon")
			return;

		EGeometryType Geometrytype = GetEnumValueFromString<EGeometryType>("EGeometryType", geomtype);  //Back From String!

		auto coords1 = geometry->GetArrayField("coordinates");

		switch (Geometrytype) {
			case EGeometryType::Polygon: {
				ParsePolygon(coords1, contourData);
				break;
			}
			case EGeometryType::MultiPolygon: {
				ParseMultiPolygon(coords1, contourData);
				break;
			}
		}

		ContoursWithData.Add(contourData);
	}
}

void AJsonLoader::ParsePolygon(TArray<TSharedPtr<FJsonValue>> geometry, FPosgisContourData& contourData)
{
	FContour contour;
	auto coords2 = geometry[0]->AsArray();
	TArray<FVector> points;
	FGeoCoords geoCoords = FGeoCoords(_projection, _originLon, _originLat);
	for (int i = 0; i < coords2.Num(); i++)
	{
		auto coords3 = coords2[i]->AsArray();
		points.Add(UGeoHelpers::GetLocalCoordinates(coords3[0]->AsNumber(), coords3[1]->AsNumber(), 0, geoCoords));
	}
	contour.Points = points;

	contourData.Outer.Add(contour);

	contour = FContour();
	for (int i = 1; i < geometry.Num(); i++)
	{
		coords2 = geometry[i]->AsArray();

		TArray<FVector> points2;
		for (int j = 0; j < coords2.Num(); j++)
		{
			auto coords3 = coords2[j]->AsArray();
			points2.Add(UGeoHelpers::GetLocalCoordinates(coords3[0]->AsNumber(), coords3[1]->AsNumber(), 0, geoCoords));
		}
		contour.Points = points2;
	}
	if (contour.Points.Num()>0)
	{
		contourData.Holes.Add(contour);
	}
}

void AJsonLoader::ParseMultiPolygon(TArray<TSharedPtr<FJsonValue>> geometry, FPosgisContourData& contourData)
{
	for (auto geom : geometry)
	{
		ParsePolygon(geom->AsArray(), contourData);
	}
}

void AJsonLoader::ParseRecursion(TMap<FString, TSharedPtr<FJsonValue>> Values)
{
	for (auto val : Values)
	{
		JSONString += TabString + "\"" + val.Key + "\":";

		if (val.Value->Type == EJson::Object)
		{
			JSONString += "\n" + TabString + "{\n";
			TabString += "\t";
			ParseRecursion(val.Value->AsObject()->Values);
			TabString.RemoveFromEnd("\t");
			JSONString += "\n" + TabString + "}";
		} 
		else if (val.Value->Type == EJson::Array)
		{
			JSONString += "\n" + TabString + "[\n";
			TabString += "\t";
			ParseArray(val.Value->AsArray());
			TabString.RemoveFromEnd("\t");
			JSONString += "\n" + TabString + "]";
		}
		else{
			JSONString += " \"" + val.Value->AsString() + "\"";
		}
		JSONString += ",\n";
	}
	JSONString.RemoveFromEnd(",\n");
}

void AJsonLoader::ParseArray(TArray<TSharedPtr<FJsonValue>> Values)
{
	for (auto val : Values)
	{
		if (val->Type == EJson::Object)
		{
			JSONString += "\n" + TabString + "{\n";
			TabString += "\t";
			ParseRecursion(val->AsObject()->Values);
			TabString.RemoveFromEnd("\t");
			JSONString += "\n" + TabString + "}";
		}
		else if (val->Type == EJson::Array)
		{
			JSONString += "\n" + TabString + "[\n" + TabString;
			TabString += "\t";
			ParseArray(val->AsArray());
			TabString.RemoveFromEnd("\t");
			JSONString += "\n" + TabString + "]";
		}
		else {
			JSONString += " \"" + val->AsString() + "\"";
		}
		JSONString += ", ";
	}
	JSONString.RemoveFromEnd(", ");
}
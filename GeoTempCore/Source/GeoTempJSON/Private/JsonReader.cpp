#include "JsonReader.h"


TArray<FMultipolygonData> UJsonReader::ReadContoursFromFile(FString inFilepath, FGeoCoords inGeoCoords)
{
	const FString jsonFilePath = inFilepath;
	FString jsonString;
	FFileHelper::LoadFileToString(jsonString, *jsonFilePath);

	return ReadContoursFromString(jsonString, inGeoCoords);
}


TArray<FMultipolygonData> UJsonReader::ReadContoursFromString(FString inJsonString, FGeoCoords inGeoCoords)
{
	TSharedPtr<FJsonObject>		jsonObject = MakeShareable(new FJsonObject());
	TSharedRef<TJsonReader<>>	jsonReader = TJsonReaderFactory<>::Create(inJsonString);

	TArray<FMultipolygonData> contoursWithData;
	if (FJsonSerializer::Deserialize(jsonReader, jsonObject))
	{
		contoursWithData = ReadContoursFromJSON(jsonObject, inGeoCoords);
	}

	return contoursWithData;
}


TArray<FMultipolygonData> UJsonReader::ReadContoursFromJSON(TSharedPtr<FJsonObject> inJsonObject, FGeoCoords inGeoCoords)
{
	GeoCoords = inGeoCoords;

	TArray<FMultipolygonData> contoursWithData;
	auto featureArray = inJsonObject->GetArrayField("features");
	ParseFeatures(featureArray, contoursWithData);
	return contoursWithData;
}


void UJsonReader::ParseJSON()
{
	const FString jsonFilePath = FPaths::ProjectContentDir() + "Buildings Layer.geojson";
	FString jsonString;

	FFileHelper::LoadFileToString(jsonString, *jsonFilePath);

	TSharedPtr<FJsonObject>		jsonObject = MakeShareable(new FJsonObject());
	TSharedRef<TJsonReader<>>	jsonReader = TJsonReaderFactory<>::Create(jsonString);

	if (FJsonSerializer::Deserialize(jsonReader, jsonObject))
	{
		TArray<JsonValuesPtr> FeatureArray = jsonObject->GetArrayField("features");
		JsonString	+= "{\n";
		TabString	+= "\t";
		ParseRecursion(jsonObject->Values);
		JsonString	+= "}";
	}
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

		auto	geometry = feature->AsObject()->GetObjectField("geometry");
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
		points.Add(UGeoHelpers::GetLocalCoordinates(pointCoords[0]->AsNumber(), pointCoords[1]->AsNumber(), 0, GeoCoords));
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
			holesPoints.Add(UGeoHelpers::GetLocalCoordinates(pointCoords[0]->AsNumber(), pointCoords[1]->AsNumber(), 0, GeoCoords));
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
		JsonString += TabString + "\"" + val.Key + "\":";

		switch (val.Value->Type)
		{
			case EJson::Object:
				JsonString	+= "\n" + TabString + "{\n";
				TabString	+= "\t";
				ParseRecursion(val.Value->AsObject()->Values);
				TabString.RemoveFromEnd("\t");
				JsonString	+= "\n" + TabString + "}";
				break;

			case EJson::Array:
				JsonString	+= "\n" + TabString + "[\n";
				TabString	+= "\t";
				ParseArray(val.Value->AsArray());
				TabString.RemoveFromEnd("\t");
				JsonString	+= "\n" + TabString + "]";
				break;

			default:
				JsonString	+= " \"" + val.Value->AsString() + "\"";
		}

		JsonString += ",\n";
	}
	JsonString.RemoveFromEnd(",\n");
}


void UJsonReader::ParseArray(TArray<JsonValuesPtr> inValues)
{
	for (auto val : inValues)
	{
		switch (val->Type)
		{
			case EJson::Object:
				JsonString	+= "\n" + TabString + "{\n";
				TabString	+= "\t";
				ParseRecursion(val->AsObject()->Values);
				TabString.RemoveFromEnd("\t");
				JsonString	+= "\n" + TabString + "}";
				break;

			case EJson::Array:
				JsonString	+= "\n" + TabString + "[\n" + TabString;
				TabString	+= "\t";
				ParseArray(val->AsArray());
				TabString.RemoveFromEnd("\t");
				JsonString	+= "\n" + TabString + "]";
				break;

			default:
				JsonString	+= " \"" + val->AsString() + "\"";
		}

		JsonString += ", ";
	}
	JsonString.RemoveFromEnd(", ");
}

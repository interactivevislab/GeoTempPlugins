#include "BasePolygonPreparer.h"


void UBasePolygonPreparer::PrepareMaskLoader(UMaskLoader* target, TArray<FPosgisContourData> polygons, TMap<FString, FString> tags)
{
	if (polygons.Num() == 0) return;

	FString StartAppearTag = *tags.Find("AppearStart");
	FString EndAppearTag = *tags.Find("AppearEnd");
	FString StartDemolishTag = *tags.Find("DemolishStart");
	FString EndDemolishTag = *tags.Find("DemolishEnd");
	FString ExcludeTag = *tags.Find("Exclude");
	FString ExcludeValue = *tags.Find("ExcludeValue");
	FString AltTag = *tags.Find("Alt");
	FString AltValue = *tags.Find("AltValue");

	float minX, maxX, minY, maxY;
	minX = minY = MAX_FLT;
	maxX = maxY = MAX_FLT * -1.0f;
	target->Vertices.Empty();
	target->ExclVertices.Empty();
	target->Triangles.Empty();
	target->ExclTriangles.Empty();
	int polyNumber = 0;
	for (auto polygon : polygons)
	{
		std::vector<FVector> points;
		std::vector<int> triangles;
		Triangulate(polygon.Outer, polygon.Holes, points, triangles);

		auto excludeValue = polygon.Tags.Find(ExcludeTag);
		bool isExclude = excludeValue && ExcludeValue.Equals(*excludeValue);

		auto altValue = polygon.Tags.Find(AltTag);
		bool isAlt = altValue && AltValue.Equals(*altValue);

		auto& Vertices = isExclude ? target->ExclVertices : target->ExclVertices;
		auto& Triangles = isExclude ? target->ExclTriangles : target->ExclTriangles;

		auto startAppearValue = polygon.Tags.Find(StartAppearTag);
		int startAppearYear = !startAppearValue ? 0 : FCString::Atoi(**startAppearValue);
		auto endAppearValue = polygon.Tags.Find(EndAppearTag);
		int endAppearYear = !endAppearValue ? startAppearYear + 1 : FCString::Atoi(**endAppearValue);
		if (endAppearYear - startAppearYear <= 0) endAppearYear = startAppearYear + 1;
		auto startDemolishValue = polygon.Tags.Find(StartDemolishTag);
		int startDemolishYear = !startDemolishValue ? 3000 : FCString::Atoi(**startDemolishValue);
		auto endDemolishValue = polygon.Tags.Find(EndDemolishTag);
		int endDemolishYear = !endDemolishValue ? startDemolishYear + 1 : FCString::Atoi(**endDemolishValue);
		if (endDemolishYear - startDemolishYear <= 0) endDemolishYear = startDemolishYear + 1;
		int zeroInd = Vertices.Num();


		for (int i = 0; i < points.size(); i++)
		{
			minX = FMath::Min(minX, points[i].X);
			maxX = FMath::Max(maxX, points[i].X);
			minY = FMath::Min(minY, points[i].Y);
			maxY = FMath::Max(maxY, points[i].Y);

			FMyTextureVertex vert;
			vert.Position = points[i] +
				(isExclude ? FVector::UpVector * 1 : FVector::ZeroVector);
			vert.Color = (isExclude ? FColor(0, !isAlt ? 255 : 0, 0, isAlt ? 255 : 0) : FColor(!isAlt ? 255 : 0, 0, isAlt ? 255 : 0, 0));
			vert.YearData = FVector4(startAppearYear, endAppearYear, startDemolishYear, endDemolishYear);
			Vertices.Add(vert);
		}

		for (int i = 0; i < triangles.size(); i++)
		{
			Triangles.Add(triangles[i] + zeroInd);
		}
		polyNumber++;

		target->Years.AddUnique(startAppearYear);
		target->Years.AddUnique(endAppearYear);
		target->Years.AddUnique(startDemolishYear);
		target->Years.AddUnique(endDemolishYear);
	}
	target->Years.Sort();
	if (maxX - minX < maxY - minY) maxX = minX + (maxY - minY);
	else maxY = minY + (maxX - minX);
	target->Rect = FVector4(minX, maxX, minY, maxY);
	target->UpdateRect();
	target->Dirty = true;
}

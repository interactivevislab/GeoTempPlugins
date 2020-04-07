#include "ZonesPolygonPreparer.h"


void UZonesPolygonPreparer::PrepareMaskLoader(UMaskLoader* inTarget, TArray<FPosgisContourData> inPolygonData,
	TMap<FString, FString> inTags)
{
	if (inPolygonData.Num() == 0)
	{
		return;
	}

	FString startAppearTag		= *inTags.Find("AppearStart");
	FString endAppearTag		= *inTags.Find("AppearEnd");
	FString startDemolishTag	= *inTags.Find("DemolishStart");
	FString endDemolishTag		= *inTags.Find("DemolishEnd");
	FString categoryTag			= *inTags.Find("Category");
	FString mask1Tag			= *inTags.Find("MaskCategoryR");
	FString mask2Tag			= *inTags.Find("MaskCategoryG");
	FString mask3Tag			= *inTags.Find("MaskCategoryB");
	FString mask4Tag			= *inTags.Find("MaskCategoryA");

	float minX, maxX, minY, maxY;
	minX = minY = MAX_FLT;
	maxX = maxY = MAX_FLT * -1.0f;
	inTarget->Vertices.Empty();
	inTarget->Triangles.Empty();

	for (auto polygon : inPolygonData)
	{
		TArray<FVector> points;
		TArray<int> triangles;
		Triangulate(polygon.Outer, polygon.Holes, points, triangles);

		auto CatValue = polygon.Tags.Find(categoryTag);

		FColor color = FColor::Black;

		if (CatValue->Equals(mask1Tag))
		{
			color = FColor(255, 0, 0, 0);
		}
		else if (CatValue->Equals(mask2Tag))
		{
			color = FColor(0, 255, 0, 0);
		}
		else if (CatValue->Equals(mask3Tag))
		{
			color = FColor(0, 0, 255, 0);
		}
		else if (CatValue->Equals(mask4Tag))
		{
			color = FColor(0, 0, 0, 255);
		}
		else
		{
			continue;
		}

		auto startAppearValue	= polygon.Tags.Find(startAppearTag);
		auto endAppearValue		= polygon.Tags.Find(endAppearTag);
		auto startDemolishValue	= polygon.Tags.Find(startDemolishTag);
		auto endDemolishValue	= polygon.Tags.Find(endDemolishTag);

		const int LAST_YEAR = 3000;
		int startAppearYear		= startAppearValue		? FCString::Atoi(**startAppearValue)	: 0;
		int endAppearYear		= endAppearValue		? FCString::Atoi(**endAppearValue)		: startAppearYear + 1;
		int startDemolishYear	= startDemolishValue	? FCString::Atoi(**startDemolishValue)	: LAST_YEAR;
		int endDemolishYear		= endDemolishValue		? FCString::Atoi(**endDemolishValue)	: startDemolishYear + 1;

		if (endAppearYear <= startAppearYear)
		{
			endAppearYear = startAppearYear + 1;
		}
		if (endDemolishYear <= startDemolishYear)
		{
			endDemolishYear = startDemolishYear + 1;
		}

		int zeroInd = inTarget->Vertices.Num();

		for (int i = 0; i < points.Num(); i++)
		{
			minX = FMath::Min(minX, points[i].X);
			maxX = FMath::Max(maxX, points[i].X);
			minY = FMath::Min(minY, points[i].Y);
			maxY = FMath::Max(maxY, points[i].Y);

			FMyTextureVertex vert;
			vert.Position = points[i];
			vert.Color = color;
			vert.YearData = FVector4(startAppearYear, endAppearYear, startDemolishYear, endDemolishYear);
			inTarget->Vertices.Add(vert);
		}

		for (int i = 0; i < triangles.Num(); i++)
		{
			inTarget->Triangles.Add(triangles[i] + zeroInd);
		}

		inTarget->Years.AddUnique(startAppearYear);
		inTarget->Years.AddUnique(endAppearYear);
		inTarget->Years.AddUnique(startDemolishYear);
		inTarget->Years.AddUnique(endDemolishYear);
	}

	inTarget->Years.Sort();

	auto dX = maxX - minX;
	auto dY = maxY - minY;
	if (dX < dY)
	{
		maxX = minX + dY;
	}
	else
	{
		maxY = minY + dX;
	}

	inTarget->Rect = FVector4(minX, maxX, minY, maxY);
	inTarget->UpdateRect();
	inTarget->Dirty = true;
}

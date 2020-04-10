#pragma once

#include "CoreMinimal.h"

#include "BasePolygonPreparer.h"

#include "ZonesPolygonPreparer.generated.h"


UCLASS(BlueprintType)
class POLYGONMASKSGENERATOR_API UZonesPolygonPreparer : public UBasePolygonPreparer
{
	GENERATED_BODY()
	
	void PrepareMaskLoader(UMaskLoader* inTarget, TArray<FContourData> inPolygonData,
		TMap<FString, FString> inTags) override;
};

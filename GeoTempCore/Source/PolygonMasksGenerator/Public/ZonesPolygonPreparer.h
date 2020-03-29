#pragma once

#include "CoreMinimal.h"
#include "BasePolygonPreparer.h"
#include "ZonesPolygonPreparer.generated.h"


UCLASS(BlueprintType)
class POLYGONMASKSGENERATOR_API UZonesPolygonPreparer : public UBasePolygonPreparer
{
	GENERATED_BODY()
	
	void PrepareMaskLoader(UMaskLoader* target, TArray<FPosgisContourData> polygonData, TMap<FString, FString> tags) override;
};

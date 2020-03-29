#pragma once

#include "CoreMinimal.h"
#include "BasePolygonPreparer.h"
#include "TreeTypesPolygonPreparer.generated.h"


UCLASS(BlueprintType)
class POLYGONMASKSGENERATOR_API UTreeTypesPolygonPreparer : public UBasePolygonPreparer
{
	GENERATED_BODY()
	
	void PrepareMaskLoader(UMaskLoader* target, TArray<FPosgisContourData> polygonData, TMap<FString, FString> tags) override;
};

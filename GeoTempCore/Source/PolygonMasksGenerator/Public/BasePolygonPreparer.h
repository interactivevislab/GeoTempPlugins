#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "MasksLoader.h"

#include "BasePolygonPreparer.generated.h"


UCLASS(BlueprintType)
class POLYGONMASKSGENERATOR_API UBasePolygonPreparer : public UObject
{
	GENERATED_BODY()
	
	UFUNCTION(BlueprintCallable)
	virtual void PrepareMaskLoader(UMaskLoader* target, TArray<FPosgisContourData> polygonData, TMap<FString, FString> tags);
};

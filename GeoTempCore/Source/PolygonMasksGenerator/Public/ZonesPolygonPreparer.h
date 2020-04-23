#pragma once

#include "CoreMinimal.h"

#include "BasePolygonPreparer.h"

#include "ZonesPolygonPreparer.generated.h"

/** Overload of Polygon preparer for loading of functional zones */
UCLASS(BlueprintType)
class POLYGONMASKSGENERATOR_API UZonesPolygonPreparer : public UBasePolygonPreparer
{
	GENERATED_BODY()
	
	/** @name Implementation of UBasePolygonPreparer */
	///@{
	void PrepareMaskLoader(UMaskLoader* inTarget, TArray<FMultipolygonData> inPolygonData, 
		TMap<FString, FString> inTags) override;
	///@}
};

#pragma once

#include "CoreMinimal.h"

#include "BasePolygonPreparer.h"

#include "TreeTypesPolygonPreparer.generated.h"

/** Overload of Polygon preparer for loading of Tree types and density */
UCLASS(BlueprintType)
class POLYGONMASKSGENERATOR_API UTreeTypesPolygonPreparer : public UBasePolygonPreparer
{
    GENERATED_BODY()
    
    /** @name Implementation of UBasePolygonPreparer */
    ///@{
    void PrepareMaskLoader(UMaskLoader* inTarget, const TArray<FMultipolygonData>& inPolygonData,
            const TMap<FString, FString>& inTags) override;
    ///@}
};

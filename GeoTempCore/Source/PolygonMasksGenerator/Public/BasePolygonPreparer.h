#pragma once

#include "CoreMinimal.h"

#include "UObject/NoExportTypes.h"

#include "MasksLoader.h"

#include "BasePolygonPreparer.generated.h"


/** \class UBasePolygonPreparer
 * \brief Base class for preparing Mask Loader for making masks with polygons
 * 
 * This class contains rules for rendering of input polygons and processes input to create vertex and triangle buffers for MaskLoader
 * @see MaskLoader
 */
UCLASS(BlueprintType)
class POLYGONMASKSGENERATOR_API UBasePolygonPreparer : public UObject
{
	GENERATED_BODY()

	/** Prepare mask loader with polygons and tag mapping*/
	UFUNCTION(BlueprintCallable)
	virtual void PrepareMaskLoader(UMaskLoader* inTarget, TArray<FMultipolygonData> inPolygonData,
		TMap<FString, FString> inTags);
};

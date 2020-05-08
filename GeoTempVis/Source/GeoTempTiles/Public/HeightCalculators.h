#pragma once

#include "TilesBasics.h"
#include "HeightCalculators.generated.h"


/** Implemetation of IHeightCalculator to handle elevation of MapBox heightmaps */
UCLASS(BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UMapBoxHeightCalculator : public UObject, public IHeightCalculator
{
public:

	GENERATED_BODY()

	/** @name Implementation of IHeightCalculator */
	///@{
	float CalcHeight_Implementation(FColor inColor) override
	{
		return 100 * (-10000 + (inColor.R * 256 * 256 + inColor.G * 256 + inColor.B) * 0.1f);
	}
	///@}
};
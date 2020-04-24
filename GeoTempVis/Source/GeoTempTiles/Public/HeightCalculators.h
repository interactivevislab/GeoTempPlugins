#pragma once

#include "TileGeometryGenerator.h"

#include "HeightCalculators.generated.h"


UCLASS(BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UMapBoxHeightCalculator : public UObject, public IHeightCalculator
{
public:

	GENERATED_BODY()	
	float CalcHeight_Implementation(FColor color) override
	{
		return 100 * (-10000 + color.R * 256 * 256 + color.G * 256 + color.B);
	}
};
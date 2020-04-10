#pragma once

#include "CoreMinimal.h"

#include "UObject/Interface.h"

#include "GeometryData.h"

#include "ILoaderFolliage.generated.h"


UINTERFACE(BlueprintType)
class GEOTEMPLOADERS_API ULoaderFolliage : public UInterface
{
	GENERATED_BODY()
};


class GEOTEMPLOADERS_API ILoaderFolliage
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	TArray<FContourData> GetFolliage();
};

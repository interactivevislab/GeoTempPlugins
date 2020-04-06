#pragma once

#include "CoreMinimal.h"

#include "UObject/Interface.h"

#include "IReaderOsm.generated.h"


class UOsmReader;


UINTERFACE(BlueprintType)
class GEOTEMPOSM_API UReaderOsm : public UInterface
{
	GENERATED_BODY()
};


class GEOTEMPOSM_API IReaderOsm
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SetOsmReader(UOsmReader* inOsmReader);
};
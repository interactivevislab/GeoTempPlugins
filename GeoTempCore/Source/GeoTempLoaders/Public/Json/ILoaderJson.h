#pragma once

#include "CoreMinimal.h"

#include "UObject/Interface.h"

#include "JsonReader.h"

#include "ILoaderJson.generated.h"


UINTERFACE(BlueprintType)
class GEOTEMPLOADERS_API ULoaderJson : public UInterface
{
	GENERATED_BODY()
};


class GEOTEMPLOADERS_API ILoaderJson
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SetJsonReader(AJsonReader* inJsonReader);
};

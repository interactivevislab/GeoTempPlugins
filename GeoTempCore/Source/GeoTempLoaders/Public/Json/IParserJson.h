#pragma once

#include "CoreMinimal.h"

#include "UObject/Interface.h"

#include "JsonReader.h"

#include "IParserJson.generated.h"


UINTERFACE(BlueprintType)
class GEOTEMPLOADERS_API UParserJson : public UInterface
{
	GENERATED_BODY()
};


class GEOTEMPLOADERS_API IParserJson
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SetJsonReader(UJsonReader* inJsonReader);
};

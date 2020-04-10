#pragma once

#include "CoreMinimal.h"

#include "IParserOsm.h"
#include "ProvidersInterfaces/IProviderBuildings.h"

#include "LoaderBuildingsOsm.generated.h"


UCLASS(Blueprintable, meta=(BlueprintSpawnableComponent))
class GEOTEMPLOADERS_API ULoaderBuildingsOsm : public UObject, public IParserOsm, public IProviderBuildings
{
	GENERATED_BODY()

public:

	virtual void SetOsmReader_Implementation(UOsmReader* inOsmReader) override;
	virtual TArray<FBuilding> GetBuildings_Implementation() override;

private:

	UOsmReader* osmReader;
};

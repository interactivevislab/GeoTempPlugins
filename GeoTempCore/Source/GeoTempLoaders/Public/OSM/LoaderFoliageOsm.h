#pragma once

#include "CoreMinimal.h" //#include "Components/ActorComponent.h"

#include "IParserOsm.h"
#include "ProvidersInterfaces/IProviderFolliage.h"
#include "OSMReader.h"
#include "GeometryData.h"
#include "LoaderFoliageOsm.generated.h"


class UOsmReader;


UCLASS(Blueprintable)
class ULoaderFoliageOsm : public UObject, public IParserOsm, public IProviderFolliage
{
	GENERATED_BODY()

public:

	virtual void SetOsmReader_Implementation(UOsmReader* inOsmReader) override;
	virtual TArray<FMultipolygonData> GetFolliage_Implementation() override;

private:

	UPROPERTY()
	UOsmReader* osmReader;

	//UFUNCTION(BlueprintCallable, BlueprintPure)
//	TArray<FContourData> GetFoliage();
};

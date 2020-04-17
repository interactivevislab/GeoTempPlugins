#pragma once

#include "CoreMinimal.h"
#include "TilesBasics.h"
#include "ITilePreparer.generated.h"


UINTERFACE(MinimalAPI)
class UTilePreparer : public UInterface
{
public:
	GENERATED_BODY()
};

class ITilePreparer
{
public:
	GENERATED_BODY()	

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Default")
	void RequestTile(UTileData* tileInfo, UMaterialInstanceDynamic* mat, UTileTextureContainer* owner, const FString& channel);
};



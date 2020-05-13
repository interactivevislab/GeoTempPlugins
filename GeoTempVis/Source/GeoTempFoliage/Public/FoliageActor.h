#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "FoliageActor.generated.h"


/**
* \class AFoliageActor
* \brief Actor, that represent foliage.
*
* Has no special logic, created to distinguish foliage meshes from other.
* @see UCustomFoliageInstancer
*/
UCLASS(BlueprintType)
class GEOTEMPFOLIAGE_API    AFoliageActor : public AActor
{
    GENERATED_BODY()

public:
    /** Sets default values for this actor's properties. */
    AFoliageActor();

    UPROPERTY()
    USceneComponent* Root;
};

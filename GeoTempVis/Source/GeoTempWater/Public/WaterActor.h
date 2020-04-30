#pragma once
#include "CoreMinimal.h"

#include "RuntimeMeshActor.h"

#include "WaterActor.generated.h"


/**
* \class AWaterActor
* \brief Actor, that represent water.
*
* Has no special logic, created to distinguish water meshes from other.
* @see UWaterBuilder
*/
UCLASS(BlueprintType)
class GEOTEMPWATER_API	AWaterActor : public ARuntimeMeshActor
{
	GENERATED_BODY()
};

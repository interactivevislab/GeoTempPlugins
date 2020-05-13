#include "CoreMinimal.h"

#include "RuntimeMeshActor.h"

#include "RoadNetworkActor.generated.h"


/**
* \class ARoadNetworkActor
* \brief Actor, that represent road network.
*
* Has no special logic, created to distinguish road network meshes from other.
* @see URoadBuilder
*/
UCLASS(BlueprintType)
class GEOTEMPROADS_API    ARoadNetworkActor : public ARuntimeMeshActor
{
    GENERATED_BODY()
};

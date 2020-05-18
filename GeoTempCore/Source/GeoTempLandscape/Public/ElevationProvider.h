#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"


#include "ElevationProvider.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnElevationRequestCompleteDelegate, float, value);

/** Container for storing async elevation requests*/
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class GEOTEMPLANDSCAPE_API UElevationRequest : public UObject
{
    GENERATED_BODY()

public:
    /** Coordinates to request elevation*/
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tiles")
        FVector RequestPosition;

    /** Result elevation */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tiles")
        float Result;

    /** Is this request processed and complete*/
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tiles")
        bool isComplete;

    /** Callback to call when this request is complete */
    UPROPERTY(BlueprintAssignable, Category = "Tiles")
        FOnElevationRequestCompleteDelegate Callback;
};

UINTERFACE(MinimalAPI)
class UElevationProvider : public UInterface
{
public:
    GENERATED_BODY()
};


/** Interface handler for different approaches to parse elevation from pixel value*/
class IElevationProvider
{
public:
    GENERATED_BODY()

        /** Calculate height in meters based on color value*/
        UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Default")
        float TryGetElevation(FVector inCoords);

		UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Default")
            UElevationRequest* RequestElevation(FVector inCoords);
};
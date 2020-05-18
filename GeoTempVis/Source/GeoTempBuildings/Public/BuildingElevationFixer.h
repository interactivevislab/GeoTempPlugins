#pragma once 
#include "BuildingActor.h"
#include "BuildingPart.h"
#include "CoreMinimal.h"
#include <ElevationProvider.h>
#include "BuildingElevationFixer.generated.h"



UCLASS(ClassGroup = (Custom), BlueprintType)
class USinglePartFixer : public UObject
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
	UBuildingPartComponent* Part;

	static USinglePartFixer* Init(UBuildingPartComponent* inPart, TScriptInterface<IElevationProvider> provider);

	void FixBuilding();;
protected:
	TArray<UElevationRequest*> Requests;

	
	UFUNCTION()
	void ProcessRequest(float value);

	TArray<float> heights;
	int AwaitingRequestCount;

	
};

UCLASS(ClassGroup = (Custom), BlueprintType)
class GEOTEMPBUILDINGS_API UBuildingElevationFixer : public UObject
{
	GENERATED_BODY()

public:

	TArray<USinglePartFixer*> fixers;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
	TArray<ABuildingActor*> Buildings;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
		TScriptInterface<IElevationProvider> ElevationProvider;

	UFUNCTION(BlueprintCallable, Category = "Elevation")
	void FixBuildings();
};
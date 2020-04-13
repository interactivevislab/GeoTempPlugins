#pragma once
#include "Basics.h"
#include "BuildingsData.h"
#include "RuntimeMeshComponent.h"
#include "BuildingPart.generated.h"

class ABuildingActor;

/** \class UBuildingPartComponent
 *	Component that controls and visualizes single part of a building
 *	
 */
UCLASS()
class GEOTEMPBUILDINGS_API UBuildingPartComponent : public URuntimeMeshComponent
{
	GENERATED_BODY()

public:
	UBuildingPartComponent(const FObjectInitializer& ObjectInitializer);

	
	/** Material for walls (and wall-type decorations) of the building */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Default")
	UMaterialInterface* WallMaterial;

	/** Material for roofs (and roof-type decorations) of the building */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
	UMaterialInterface* RoofMaterial;

	/** List of Outer contours of part footprint */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category="Default")
	TArray<FContour> Outer;

	/** List of inner contours (aka holes) of part footprint */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category="Default")
	TArray<FContour> Inner;

	/** Floors number of this part */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Default")
	int Floors;

	/** Height of this part */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Default")
	float Height;

	/** Floor on which this part begins (useful for hanged parts or roof extensions) */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Default")
	int MinFloors;

	/** Elevation of bottom of this part (useful for hanged parts or roof extensions) */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Default")
	float MinHeight;

	/** Dates of building and demolition of this part (useful for dynamic and retrospective visualizations) */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
	FBuildingDates BuildingDates;

	/** Should this part use a height or floors number to determine its size in scene*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Default")
	bool OverrideHeight;

	/** Should this part recalculate mesh every time the `Recalc` function called (e. g. in construction script of building), or only on first initialization
	 * @see Recalc
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Default")
	bool AutoUpdateMesh = false;

	/** Id of this part */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category="Default")
	int Id;

	/** Name of palette for Wall Stacker
	 * @see UWallStacker
	 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Default")
	FString StylePalette;

	/** Center of this part's bounding box */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	FVector Center;

	/** Dictionary of tags applied to this part on load. Often includes parameters related to its appearance and functionality */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	TMap<FString, FString> Tags;

	/**Color of this part */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	FLinearColor Color;	

private:
	bool _isInit = false;

public:
	/** Function called when something changed for this part. Updates its mesh if AutoUpdateMesh is set to true
	 * @see AutoUpdateMesh */
	UFUNCTION(BlueprintCallable, CallInEditor, Category="Default")
	virtual void Recalc();

	/** \fn Init *
	 * \brief Initialize this part with data structure
	 * @param inPart structure to initialize with
	 * @see FBuildingPart
	 */
	void Init(const FBuildingPart& inPart);

	/** Reinitialize this part */
	void ForceRecalc();

	/** Data of this part */
	FBuildingPart PartData;

	/** Parent Building Actor */
	ABuildingActor* Parent;

	/** \fn CreateSimpleStructure
	 * Create simple geometry of this part
	 * @param inZeroHeight override zero elevation of a building (useful for creation of basements or placing building on uneven terrain
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Category="Default")
	virtual void CreateSimpleStructure(float inZeroHeight = 0);

	/** \fn SetHeightAlpha
	 * Update building position to render it partially submerged under ground based on input fraction (useful for dynamic and retrospective visualizations)
	 * @param inHeightAlpha fraction of building growth. 1 — full size; 0 — submerged	 
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Default")
	void SetHeightAlpha(float inHeightAlpha);

	/** \fn ApplyCurrentTime
	 * function to automatically update building appearance based on time
	 * @param inCurrentTime time to base current submerge fraction on
	 * @param inMustHide should function disable visibility for completely submerged buildings
	 * @see SetHeightAlpha
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Default")
	void ApplyCurrentTime(FDateTime inCurrentTime, bool inMustHide = false);
};

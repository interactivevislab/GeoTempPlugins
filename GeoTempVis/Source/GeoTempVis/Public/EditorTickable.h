#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameFramework/Actor.h"
#include "EditorTickable.generated.h"

UINTERFACE(MinimalAPI)
class UEditorTickable : public UInterface
{
public:
	GENERATED_BODY()
};

/** Interface for implemetning editor tick for components and objects */
class GEOTEMPVIS_API IEditorTickable
{
public:
	GENERATED_BODY()

	/** This function is called on editor tick */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Default")
	void EditorTick(float deltaTime);
};

/** Actor which handles IEditorTickable components and calls editor tick on them*/
UCLASS()
class GEOTEMPVIS_API ATickableContainer : public AActor
{
	GENERATED_BODY()

	/** List of tickable components */
	UPROPERTY()
	TArray<TScriptInterface<IEditorTickable>> Tickables;

	/** Collect all EditorTickable components on this actor */
	UFUNCTION(CallInEditor, BlueprintCallable)
	void CollectTickables();

	
	/** @name Implementation of AActor default events */
	///@{
	///
	void Tick(float DeltaSeconds) override;;
	
	
	bool ShouldTickIfViewportsOnly() const override;;

	virtual void PostLoad() override;

	///@}
};
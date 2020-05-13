#include "EditorTickable.h"

void ATickableContainer::CollectTickables()
{
    TArray<UActorComponent*> foundActors = GetComponentsByInterface(UEditorTickable::StaticClass());
    for (auto& component : foundActors)
    {
        auto interface = new TScriptInterface<IEditorTickable>(component);
        if (interface)
        {
            Tickables.Add(*interface);
        }
    }
}

void ATickableContainer::Tick(float DeltaSeconds)
{
    if (Tickables.Num() == 0) CollectTickables();
    for (auto tickable : Tickables)
    {
        if (!tickable.GetObject()) continue;
        const auto& interface = Cast<IEditorTickable>(tickable.GetObject());
        interface->Execute_EditorTick(tickable.GetObject(), DeltaSeconds);
    }
}

bool ATickableContainer::ShouldTickIfViewportsOnly() const
{
    return true;
}

void ATickableContainer::PostLoad()
{
    Super::PostLoad();
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;
    PrimaryActorTick.bTickEvenWhenPaused = true;
    Tickables.Empty();
    //CollectTickables();
}

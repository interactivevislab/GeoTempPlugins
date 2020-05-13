#include "FoliageActor.h"


AFoliageActor::AFoliageActor()
{
    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    Root->OnComponentCreated();
    Root->SetMobility(EComponentMobility::Movable);
    Root->bEditableWhenInherited = true;
    SetRootComponent(Root);
}
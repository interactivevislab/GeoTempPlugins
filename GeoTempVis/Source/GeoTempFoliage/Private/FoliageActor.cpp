#include "FoliageActor.h"


AFoliageActor::AFoliageActor()
{
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	Root->Mobility = EComponentMobility::Movable;
}
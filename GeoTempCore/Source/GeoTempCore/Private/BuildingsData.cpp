#pragma once

#include "BuildingsData.h"


FBuildingPart::FBuildingPart(const long id) : Id(id), Owner(nullptr)
{
}

FBuildingPart::FBuildingPart() : Id(0), Owner(nullptr)
{
}


FBuilding::FBuilding()
{
}


FBuilding::FBuilding(const long id) : Id(id)
{
}

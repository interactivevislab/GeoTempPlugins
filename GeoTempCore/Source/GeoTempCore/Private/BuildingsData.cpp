#pragma once

#include "BuildingsData.h"


FBuildingPart::FBuildingPart(const long id) : Id(id), Owner(nullptr) {}


FBuilding::FBuilding() {}


FBuilding::FBuilding(const long id) : Id(id) {}
#include "OSM/LoaderBuildingsOsm.h"

#include "igl/point_in_poly.h"

const FString FLOORS_TAG_STRING		= "levels";
const FString HEIGHT_TAG_STRING		= "height";
const FString MIN_FLOORS_TAG_STRING = "min_levels";
const FString MIN_HEIGHT_TAG_STRING = "min_height";
const FString COLOR_TAG_STRING		= "min_height";


const FString* FindBuildingTag(const TMap<FString, FString>& inTags, const FString& inTag, const FString& inTagPrefix = "building:")
{
	auto tag = inTags.Find(inTagPrefix + inTag);
	if (!tag)
	{
		tag = inTags.Find(inTag);
	}
	return tag;
}


void InitBuildingPart(const OsmWay* inWay, FBuildingPart& outPart)
{
	auto floorsTag = FindBuildingTag(inWay->Tags, FLOORS_TAG_STRING);
	auto heightTag = FindBuildingTag(inWay->Tags, HEIGHT_TAG_STRING);
	auto minFloorsTag = FindBuildingTag(inWay->Tags, MIN_FLOORS_TAG_STRING);
	auto minHeightTag = FindBuildingTag(inWay->Tags, MIN_HEIGHT_TAG_STRING);
	auto colorTag = FindBuildingTag(inWay->Tags, COLOR_TAG_STRING);

	outPart.Floors = floorsTag
		? FCString::Atoi(**floorsTag)
		: 1;

	outPart.Height = heightTag
		? FCString::Atoi(**heightTag) * UGeoHelpers::SCALE_MULT
		: outPart.Floors * outPart.FloorHeight + 2 * UGeoHelpers::SCALE_MULT;

	outPart.MinFloors = minFloorsTag
		? FCString::Atoi(**minFloorsTag)
		: 0;

	outPart.MinHeight = minHeightTag
		? FCString::Atoi(**minHeightTag) * UGeoHelpers::SCALE_MULT
		: outPart.MinFloors * outPart.FloorHeight;

	outPart.Color = colorTag
		? FColor::FromHex(**colorTag)
		: FLinearColor::White;

	if (heightTag || minHeightTag)
	{
		outPart.OverrideHeight = true;
	}
	outPart.Tags = inWay->Tags;
}


void InitBuildingPart(const OsmRelation* inRelation, FBuildingPart& outPart)
{
	auto floorsTag = FindBuildingTag(inRelation->Tags, FLOORS_TAG_STRING);
	auto heightTag = FindBuildingTag(inRelation->Tags, HEIGHT_TAG_STRING);
	auto minFloorsTag = FindBuildingTag(inRelation->Tags, MIN_FLOORS_TAG_STRING);
	auto minHeightTag = FindBuildingTag(inRelation->Tags, MIN_HEIGHT_TAG_STRING);

	outPart.Floors = floorsTag
		? FCString::Atoi(**floorsTag)
		: 1;

	outPart.MinFloors = minFloorsTag
		? FCString::Atoi(**minFloorsTag)
		: 0;

	outPart.Height = heightTag
		? FCString::Atoi(**heightTag) * UGeoHelpers::SCALE_MULT
		: outPart.Floors * outPart.FloorHeight;

	outPart.MinHeight = minHeightTag
		? FCString::Atoi(**minHeightTag) * UGeoHelpers::SCALE_MULT
		: outPart.MinFloors * outPart.FloorHeight;

	if (heightTag || minHeightTag)
	{
		outPart.OverrideHeight = true;
	}

	outPart.Tags = inRelation->Tags;
}


bool CheckPointInContour(FContour inContour, FVector inPoint, bool& outLaysOn)
{
	outLaysOn = false;
	std::vector<std::vector<UINT>> contourPoints;	
	for (int i = 0; i < inContour.Points.Num(); i++)
	{
		if (inContour.Points[i] == inPoint) outLaysOn = true;
		std::vector<UINT> thisPoint;
		thisPoint.push_back(UINT(inContour.Points[i].X * 100));
		thisPoint.push_back(UINT(inContour.Points[i].Y * 100));
		contourPoints.push_back(thisPoint);
	}

	return igl::point_in_poly(contourPoints, UINT(inPoint.X * 100), UINT(inPoint.Y * 100));
}

void FixPartContours(FBuildingPart& outPart)
{	
	for (auto& cont : outPart.OuterConts)
	{
		cont.FixLoop();
		cont.FixClockwise();
	}

	for (auto& cont : outPart.InnerConts)
	{
		cont.FixLoop();
		cont.FixClockwise(true);
	}

	if (outPart.Floors == 0 && outPart.Height == 0)
	{
		outPart.MinFloors = -1;
	}
}

//USTRUCT()
struct FOwnersData
{
	//FOwnersData()
	//{
	//	//Buildings = TArray<int>();
	//	//Buildings.Empty();
	//	//
	//	//WayParts = TArray<int>();
	//	//WayParts.Empty();
	//}
	
	TArray<int> Buildings;
	TArray<long> WayParts;
	TArray<long> RelParts;
};


void ULoaderBuildingsOsm::SetOsmReader_Implementation(UOsmReader* inOsmReader)
{
	osmReader = inOsmReader;
}


TArray<FBuilding> ULoaderBuildingsOsm::GetBuildings_Implementation()
{
	TArray<FBuilding> buildings;
	TMap<long, FBuildingPart> wayParts;
	TMap<long, FBuildingPart> relParts;

	buildings.Empty();

	TMap<FVector, FOwnersData> pointOwners;
	TMap<long, int> wayPartOwners;
	TMap<long, int> relPartOwners;

	TSet<long> usedPartWays, usedPartRelations;

	//find all building and building parts through ways
	for (auto wayP : osmReader->Ways)
	{
		auto way = wayP.second;
		auto buildIter = way->Tags.Find("building");
		auto partIter = way->Tags.Find("building:part");

		FBuildingPart part;
		//if this is building or part
		if (buildIter || partIter)
		{
			//create and init building part data
			part = FBuildingPart(way->Id);

			//parse heights and floor counts
			InitBuildingPart(way, part);
		
			////get all points of this way and add necessary bindings
			TArray<FVector> points;
			points.Reserve(way->Nodes.size());
			for (auto node : way->Nodes)
			{
				points.Add(node->Point);				
			}

			auto cont = FContour(points);
			part.OuterConts.Add(cont);			
			

			//if this is building also create building data
			if (buildIter)
			{
				auto building = FBuilding(way->Id);
				building.Type = TCHAR_TO_UTF8(**buildIter);
				
				building.MainPart = part;
				buildings.Add(building);

				for (auto node : way->Nodes)
				{
					if (!pointOwners.Contains(node->Point))
					{
						pointOwners.Add(node->Point, FOwnersData());
					}
					FOwnersData& d = *pointOwners.Find(node->Point);
					d.Buildings.AddUnique(buildings.Num() - 1);
				}
				
			} else
			{
				wayParts.Add(part.Id, part);
				for (auto node : way->Nodes)
				{					
					if (!pointOwners.Contains(node->Point))
					{
						pointOwners.Add(node->Point, FOwnersData());
					}
					FOwnersData& d = *pointOwners.Find(node->Point);
					d.WayParts.AddUnique(part.Id);
				}
			}
		}
	}


	//find all building part so we can use it in future parsing
	for (auto relationP : osmReader->Relations)
	{
		auto relation = relationP.second;		
		auto partIter = relation->Tags.Find("building:part");
		
		if (partIter)
		{
			FBuildingPart part = FBuildingPart(relation->Id);
			InitBuildingPart(relation, part);
			
			for (auto element : relation->WayRoles)
			{
				auto way = relation->Ways[element.first];
				if (!way)
				{
					continue;
				}				
				//if this way is also a building part, just add it to parts list (is that even possible here?)
				if (!way->Tags.Find("building:part"))
				{
					auto contour = FContour();
					for (auto node : way->Nodes)
					{
						contour.Points.Add(node->Point);
					}

					if (element.second == "outer")
					{
						contour.FixClockwise();
						part.OuterConts.Add(contour);
					}
					else if (element.second == "inner")
					{
						contour.FixClockwise(false);
						part.InnerConts.Add(contour);
					}
				} else {
					if (wayParts.Contains(element.first)) 
					{
						UE_LOG(LogTemp, Warning, TEXT("Building loader unhandled case: part of a part"));
					}
				}
			}
			relParts.Add(part.Id, part);
			for (auto element : relation->WayRoles)
			{
				auto way = relation->Ways[element.first];
				if (!way)
				{
					continue;
				}
								
				for (auto node : way->Nodes)
				{					
					if (!pointOwners.Contains(node->Point))
					{
						pointOwners.Add(node->Point, FOwnersData());
					}
					FOwnersData& d = *pointOwners.Find(node->Point);
					d.RelParts.AddUnique(part.Id);
				}
			}
		}
	}

	//now process buildings
	for (auto relationP : osmReader->Relations)
	{
		auto relation = relationP.second;
		auto buildIter = relation->Tags.Find("building");
		//if this relation is building
		if (buildIter)
		{
			//create building entry
			auto building = FBuilding(relation->Id);
			building.Parts.Empty();
			building.Type = TCHAR_TO_UTF8(**buildIter);


			//create building part data from relation (it will be the footprint)
			FBuildingPart part = FBuildingPart(relation->Id);
			InitBuildingPart(relation, part);
			
			//now iterate over the ways in this relation
			for (auto element : relation->WayRoles)
			{				
				auto way = relation->Ways[element.first];				
				if (!way)
				{
					continue;
				}
				if (way->Tags.Find("building:part"))
				{
					building.Parts.Add(wayParts[element.first]);
					usedPartWays.Add(element.first);
					wayPartOwners.Add(element.first, buildings.Num());
				} else {
					auto contour = FContour();
					for (auto node : way->Nodes)
					{
						contour.Points.Add(node->Point);
					}

					if (element.second == "outer")
					{
						contour.FixClockwise();
						part.OuterConts.Add(contour);
					}
					else if (element.second == "inner")
					{
						contour.FixClockwise(true);
						part.InnerConts.Add(contour);
					}
				}
			}
			

			for (auto element : relation->RelRoles)
			{
				auto rel = relation->Relations[element.first];
				if (rel->Tags.Find("building:part"))
				{
					building.Parts.Add(relParts[element.first]);
					usedPartRelations.Add(element.first);
					relPartOwners.Add(element.first, buildings.Num());
				}
			}			
			building.MainPart = part;
			buildings.Add(building);

			for (auto element : relation->WayRoles)
			{
				auto way = relation->Ways[element.first];
				if (!way)
				{
					continue;
				}
								
				for (auto node : way->Nodes)
				{					
					if (!pointOwners.Contains(node->Point))
					{
						pointOwners.Add(node->Point, FOwnersData());
					}
					FOwnersData& d = *pointOwners.Find(node->Point);
					d.Buildings.AddUnique(buildings.Num() - 1);
				}
			}
			
		}
	}

	
	//find lost parts
	TArray<TTuple<long, FBuildingPart>> partsQueue1;
	TArray<TTuple<long, FBuildingPart>> partsQueue2;

	TArray<TTuple<long, FBuildingPart>>* partsQueue = &partsQueue1;
	TArray<TTuple<long, FBuildingPart>>* otherQueue = &partsQueue2;
	for (auto& kv : wayParts)
	{
		otherQueue->Add(kv);
	}
	bool processed = true;
	while (otherQueue->Num() != 0 && processed)
	{

		auto p = otherQueue;
		otherQueue = partsQueue;
		partsQueue = p;
		otherQueue->Empty();
		processed = false;
		
		for (int i = 0; i < partsQueue->Num(); i++) 
		{
			auto kv = (*partsQueue)[i];
			if (usedPartWays.Contains(kv.Key)) continue;

			auto& part = kv.Value;

			bool parentFound = false;

			//find building footprints including this part points
			for (auto cont : part.OuterConts)
			{
				for (auto point : cont.Points)
				{
					auto& owners = pointOwners[point];
					int foundInd = -1;
					for (int j = 0; j < owners.Buildings.Num(); j++) 
					{
						bool inside = false;
						bool laysOn = true;
						auto& building = buildings[owners.Buildings[j]];
						
						for (auto buildingContour : building.MainPart.OuterConts)
						{
							for (auto otherPoint : cont.Points) 
							{
								bool poinLaysOn = false;
								if (CheckPointInContour(buildingContour, otherPoint, poinLaysOn))
								{
									inside = true;
									break;
								}
								laysOn &= poinLaysOn;
							}						
							if (inside) break;
						}
						if (inside || laysOn) 
						{
							building.Parts.Add(part);
							wayPartOwners.Add(kv.Key, owners.Buildings[j]);
							processed = true;
							parentFound = true;
							break;
						}
						if (parentFound) break;
					}				
				}
				if (parentFound) break;
			}

			for (auto cont : part.OuterConts)
			{
				for (auto point : cont.Points)
				{
					auto& owners = pointOwners[point];
					for (auto partId : owners.WayParts)
					{					
						if (wayPartOwners.Contains(partId))
						{
							buildings[wayPartOwners[partId]].Parts.Add(part);
							wayPartOwners.Add(kv.Key, wayPartOwners[partId]);
							processed = true;
							parentFound = true;
							break;
						}
					}
					if (parentFound) break;
					for (auto partId : owners.RelParts)
					{					
						if (relPartOwners.Contains(partId))
						{
							buildings[relPartOwners[partId]].Parts.Add(part);
							wayPartOwners.Add(kv.Key, relPartOwners[partId]);
							processed = true;
							parentFound = true;
							break;
						}
					}
					if (parentFound) break;
				}
			}
			if (!parentFound) 
			{
				UE_LOG(LogTemp, Warning, TEXT("Building loader cannot find owner of a part: %i"), kv.Key);
				otherQueue->Add(kv);
			}
		}
	}
	
	
	for (auto& building : buildings)
	{
		FixPartContours(building.MainPart);
		for (auto& part : building.Parts)
		{
			FixPartContours(part);
		}
	}
	return buildings;
}

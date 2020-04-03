#include "OSM/OSMBuildingLoader.h"

TArray<FBuilding> UOsmBuildingLoader::GetBuildings(UOsmReader* source)
{
	TArray<FBuilding> buildings;
	TArray<FBuildingPart> Parts;
	
	//ways that are parts of buildings
	std::unordered_map<long, FBuildingPart*> buildingWayParts;
	//relations that are parts of building
	std::unordered_map<long, FBuildingPart*> buildingRelParts;

	buildings.Empty();

	//find all building and building parts through ways
	for (auto wayP : source->Ways)
	{
		auto way = wayP.second;
		auto buildIter = way->Tags.Find("building");
		auto partIter = way->Tags.Find("building:part");

		FBuildingPart* part;
		//if this is building or part
		if (buildIter || partIter)
		{
			//create and init building part data
			part = new FBuildingPart(way->Id);

			//parse heights and floor counts
			InitBuildingPart(way, part);

			Parts.Add(*part);
			//if this is building also create building data
			if (buildIter)
			{
				auto building = new FBuilding(way->Id);
				building->Type = TCHAR_TO_UTF8(**buildIter);

				part->Owner = building;
				building->MainPart = part;
				buildings.Add(*building);
			}

			////get all points of this way and add necessary bindings
			std::vector<FVector> points;

			for (auto node : way->Nodes)
			{
				points.push_back(node->Point);
			}

			auto cont = FContour(points);
			part->OuterConts.Add(cont);

			//add part to list
			buildingWayParts.insert_or_assign(way->Id, part);
		}
	}

	for (auto relationP : source->Relations)
	{
		auto relation = relationP.second;
		auto buildIter = relation->Tags.Find("building");
		auto partIter = relation->Tags.Find("building:part");
		FBuildingPart* part;

		//if this relation is building
		if (buildIter)
		{
			//create building entry
			auto building = new FBuilding(relation->Id);
			building->Parts.clear();
			building->Type = TCHAR_TO_UTF8(**buildIter);


			//create building part data from relation (it will be the footprint)
			part = new FBuildingPart(relation->Id);
			InitBuildingPart(relation, part);
			building->MainPart = part;

			part->Owner = building;

			//now iterate over the ways in this relation
			for (auto element : relation->WayRoles)
			{
				auto way = relation->Ways[element.first];
				if (!way)
				{
					continue;
				}

				auto& currentPart = part;
				auto c = new FContour();
				for (auto node : way->Nodes)
				{
					c->Points.Add(node->Point);
				}

				if (element.second == "outer")
				{
					c->FixClockwise();
					currentPart->OuterConts.Add(*c);
				}
				else if (element.second == "inner")
				{
					c->FixClockwise(true);
					currentPart->InnerConts.Add(*c);
				}
			}

			for (auto element : relation->RelRoles)
			{
				auto rel = relation->Relations[element.first];
				auto& currentPart = part;
				if (rel->Tags.Find("building:part"))
				{
					building->Parts.push_back(&Parts[element.first]);
				}
			}
			buildings.Add(*building);
		}

			//if this relation is building part
		else if (partIter)
		{
			part = new FBuildingPart(relation->Id);
			InitBuildingPart(relation, part);
			Parts.Add(*part);
			for (auto element : relation->WayRoles)
			{
				auto way = relation->Ways[element.first];
				if (!way)
				{
					continue;
				}
				auto& currentPart = part;
				//if this way is building part, just add it to parts list (we probably have created it already
				if (!way->Tags.Find("building:part"))
				{
					auto c = new FContour();
					for (auto node : way->Nodes)
					{
						c->Points.Add(node->Point);
					}

					if (element.second == "outer")
					{
						c->FixClockwise();
						currentPart->OuterConts.Add(*c);
					}
					else if (element.second == "inner")
					{
						c->FixClockwise(false);
						currentPart->InnerConts.Add(*c);
					}
				}
			}
		}
	}
	return buildings;
}

inline const FString* FindBuildingTag(const TMap<FString, FString>& inTags, FString inTag, FString inTagPrefix = "building:")
{
	auto tag = inTags.Find(inTagPrefix + inTag);
	if (!tag)
	{
		tag = inTags.Find(inTag);
	}
	return tag;
}


void UOsmBuildingLoader::InitBuildingPart(const OsmWay* inWay, FBuildingPart* inPart)
{
	auto floorsTag = FindBuildingTag(inWay->Tags, "levels");
	inPart->Floors = floorsTag ? FCString::Atoi(**floorsTag) : 1;

	auto heightTag = FindBuildingTag(inWay->Tags, "height");
	inPart->Height = heightTag
		? FCString::Atoi(**heightTag) * UGeoHelpers::SCALE_MULT
		: inPart->Floors * inPart->FloorHeight + 2 * UGeoHelpers::SCALE_MULT;

	auto minFloorsTag = FindBuildingTag(inWay->Tags, "min_levels");
	inPart->MinFloors = minFloorsTag ? FCString::Atoi(**minFloorsTag) : 0;

	auto minHeightTag = FindBuildingTag(inWay->Tags, "min_height");
	inPart->MinHeight = minHeightTag
		? FCString::Atoi(**minHeightTag) * UGeoHelpers::SCALE_MULT
		: inPart->MinFloors * inPart->FloorHeight;

	if (heightTag || minHeightTag)
	{
		inPart->OverrideHeight = true;
	}
}


void UOsmBuildingLoader::InitBuildingPart(const OsmRelation* inRelation, FBuildingPart* inPart)
{
	auto floorsTag		= FindBuildingTag(inRelation->Tags, "levels");
	auto heightTag		= FindBuildingTag(inRelation->Tags, "height");
	auto minFloorsTag	= FindBuildingTag(inRelation->Tags, "min_levels");
	auto minHeightTag	= FindBuildingTag(inRelation->Tags, "min_height");
	
	inPart->Floors = floorsTag
		? FCString::Atoi(**floorsTag)
		: 1;

	inPart->MinFloors = minFloorsTag
		? FCString::Atoi(**minFloorsTag)
		: 0;
	
	inPart->Height = heightTag
		? FCString::Atoi(**heightTag) * UGeoHelpers::SCALE_MULT
		: inPart->Floors * inPart->FloorHeight;
	
	inPart->MinHeight = minHeightTag
		? FCString::Atoi(**minHeightTag) * UGeoHelpers::SCALE_MULT
		: inPart->MinFloors * inPart->FloorHeight;

	if (heightTag || minHeightTag)
	{
		inPart->OverrideHeight = true;
	}
}

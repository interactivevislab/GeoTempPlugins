#include "OSM/OSMBuildingLoader.h"

inline void FixPartContours(FBuildingPart& outPart)
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

TArray<FBuilding> UOsmBuildingLoader::GetBuildings(UOsmReader* inSource)
{
	TArray<FBuilding> buildings;
	TArray<FBuildingPart> Parts;

	buildings.Empty();

	//find all building and building parts through ways
	for (auto wayP : inSource->Ways)
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

			Parts.Add(part);

			//if this is building also create building data
			if (buildIter)
			{
				auto building = FBuilding(way->Id);
				building.Type = TCHAR_TO_UTF8(**buildIter);
				
				building.MainPart = part;
				buildings.Add(building);
			}
		}
	}

	for (auto relationP : inSource->Relations)
	{
		auto relation = relationP.second;
		auto buildIter = relation->Tags.Find("building");
		auto partIter = relation->Tags.Find("building:part");
		

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

			for (auto element : relation->RelRoles)
			{
				auto rel = relation->Relations[element.first];
				if (rel->Tags.Find("building:part"))
				{
					building.Parts.Add(Parts[element.first]);
				}
			}
			buildings.Add(building);			
			building.MainPart = part;
		}

			//if this relation is building part
		else if (partIter)
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
				//if this way is building part, just add it to parts list (we probably have created it already
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
				}
			}
			Parts.Add(part);
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

inline const FString* FindBuildingTag(TMap<FString, FString> inTags, FString inTag, FString inTagPrefix = "building:")
{
	auto tag = inTags.Find(inTagPrefix + inTag);
	if (!tag)
	{
		tag = inTags.Find(inTag);
	}
	return tag;
}

FString UOsmBuildingLoader::FLOORS_TAG_STRING		= "levels";
FString UOsmBuildingLoader::HEIGHT_TAG_STRING		= "height";
FString UOsmBuildingLoader::MIN_FLOORS_TAG_STRING	= "min_levels";
FString UOsmBuildingLoader::MIN_HEIGHT_TAG_STRING	= "min_height";


void UOsmBuildingLoader::InitBuildingPart(const OsmWay* inWay, FBuildingPart& outPart)
{
	auto floorsTag		= FindBuildingTag(inWay->Tags, FLOORS_TAG_STRING);
	auto heightTag		= FindBuildingTag(inWay->Tags, HEIGHT_TAG_STRING);
	auto minFloorsTag	= FindBuildingTag(inWay->Tags, MIN_FLOORS_TAG_STRING);
	auto minHeightTag	= FindBuildingTag(inWay->Tags, MIN_HEIGHT_TAG_STRING);
	
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

	if (heightTag || minHeightTag)
	{
		outPart.OverrideHeight = true;
	}
}


void UOsmBuildingLoader::InitBuildingPart(const OsmRelation* inRelation, FBuildingPart& outPart)
{
	auto floorsTag		= FindBuildingTag(inRelation->Tags, FLOORS_TAG_STRING);
	auto heightTag		= FindBuildingTag(inRelation->Tags, HEIGHT_TAG_STRING);
	auto minFloorsTag	= FindBuildingTag(inRelation->Tags, MIN_FLOORS_TAG_STRING);
	auto minHeightTag	= FindBuildingTag(inRelation->Tags, MIN_HEIGHT_TAG_STRING);
	
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
}

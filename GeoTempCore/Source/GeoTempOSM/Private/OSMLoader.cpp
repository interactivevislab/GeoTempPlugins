#include "OSMLoader.h"
#include <algorithm>


UOsmReader::UOsmReader() : UActorComponent()
{
}


void UOsmReader::ParseBuildings()
{
	//ways that are parts of buildings
	std::unordered_map<long, FBuildingPart*> buildingWayParts;
	//relations that are parts of building
	std::unordered_map<long, FBuildingPart*> buildingRelParts;

	//list of used points ids for each way part
	std::unordered_map<long, std::vector<long>> wayPartPoints;
	//list of used points ids for each relation part
	std::unordered_map<long, std::vector<long>> relPartPoints;

	//list of containing ways ids for each points
	std::unordered_map<long, std::vector<long>> pointsWayParts;
	//list of containing relations ids for each points
	std::unordered_map<long, std::vector<long>> pointsRelParts;

	std::unordered_map<long, FBuildingPart*> wayParts;
	std::unordered_map<long, FBuildingPart*> relParts;
	Buildings.Empty();

	//find all building and building parts through ways
	for (auto wayP : Ways)
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
			if (buildIter) {
				auto building = new FBuilding(way->Id);
				building->Type = TCHAR_TO_UTF8(**buildIter);

				part->Owner = building;
				building->MainPart = part;
				//building->Parts.push_back(part);
				Buildings.Add(*building);
			}
			else
			{
				//add this part to list				
				wayParts.insert_or_assign(part->Id, part);
			}

			////get all points of this way and add necessary bindings
			std::vector<FVector> points;

			for (auto node : way->Nodes)
			{
				points.push_back(node->Point);
				pointsWayParts[node->Id].push_back(way->Id);
				wayPartPoints[way->Id].push_back(node->Id);
			}

			auto cont = FContour(points);
			part->OuterConts.Add(cont);

			//add part to list
			buildingWayParts.insert_or_assign(way->Id, part);
		}
	}

	for (auto relationP : Relations)
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
					pointsRelParts[node->Id].push_back(relation->Id);
					relPartPoints[relation->Id].push_back(node->Id);
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
			Buildings.Add(*building);
		}

		//if this relation is building part
		else if (partIter)
		{
			part = new FBuildingPart(relation->Id);
			InitBuildingPart(relation, part);
			Parts.Add(*part);
			relParts.insert_or_assign(part->Id, part);
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
						pointsRelParts[node->Id].push_back(relation->Id);
						relPartPoints[relation->Id].push_back(node->Id);
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
}


void UOsmReader::InitWithXML(FString inXmlString)
{
	XmlDocument.Parse(TCHAR_TO_UTF8(*inXmlString));
	ReadData();
}


void UOsmReader::InitWithFile(FString inFilename)
{
	XmlDocument.LoadFile(TCHAR_TO_UTF8(*inFilename));
	ReadData();
}


void UOsmReader::ReadData()
{
#pragma region Parse Nodes
	std::vector<tinyxml2::XMLElement*> nodes;

	tinyxml2::XMLNode* root = XmlDocument.FirstChild()->NextSibling();

	tinyxml2::XMLElement* bounds = root->FirstChildElement("bounds");

	double minY = bounds->DoubleAttribute("minlat");
	double maxY = bounds->DoubleAttribute("maxlat");

	double minX = bounds->DoubleAttribute("minlon");
	double maxX = bounds->DoubleAttribute("maxlon");


	tinyxml2::XMLElement* xmlNode = root->FirstChildElement("node");
	while (xmlNode)
	{
		nodes.push_back(xmlNode);
		xmlNode = xmlNode->NextSiblingElement("node");
	}
#pragma endregion

	//parse ways
#pragma region Parse Ways
	std::vector<tinyxml2::XMLElement*> ways;
	tinyxml2::XMLElement* way = root->FirstChildElement("way");
	while (way)
	{
		ways.push_back(way);
		way = way->NextSiblingElement("way");
	}
#pragma endregion

	//parse relations
#pragma region Parse Relations
	std::vector<tinyxml2::XMLElement*> relations;
	tinyxml2::XMLElement* relation = root->FirstChildElement("relation");
	while (relation)
	{
		relations.push_back(relation);
		relation = relation->NextSiblingElement("relation");
	}
#pragma endregion

	//create node objects
	for (auto node : nodes)
	{
		double lon = node->DoubleAttribute("lon");
		double lat = node->DoubleAttribute("lat");
		auto tag = node->FirstChildElement("tag");
		auto id = node->Int64Attribute("id");

		if (tag == nullptr)
		{
			auto nodeObj = new OsmNode(id, lon, lat, GeoCoords);
			Nodes.insert_or_assign(id, nodeObj);
		}
		else
		{
			auto nodeObj = new OsmNode(id, lon, lat, GeoCoords);
			while (tag != nullptr)
			{
				auto key = std::string(tag->Attribute("k"));
				std::transform(key.begin(), key.end(), key.begin(), ::tolower);
				nodeObj->Tags.Add(key.c_str(), tag->Attribute("v"));
				tag = tag->NextSiblingElement("tag");
			}
			Nodes.insert_or_assign(id, nodeObj);
		}
	}

	//create ways objects
	for (auto currentWay : ways)
	{
		auto node = currentWay->FirstChildElement("nd");
		if (!node)
		{
			continue;
		}

		auto id = currentWay->Int64Attribute("id");
		auto wayObj = new OsmWay(id);
		auto tag = currentWay->FirstChildElement("tag");

		while (tag != nullptr)
		{
			auto key = std::string(tag->Attribute("k"));
			auto value = std::string(tag->Attribute("v"));
			std::transform(key.begin(), key.end(), key.begin(), ::tolower);
			wayObj->Tags.Add(key.c_str(), tag->Attribute("v"));
			tag = tag->NextSiblingElement("tag");
		}

		Ways.insert_or_assign(id, wayObj);

		while (node != nullptr)
		{
			wayObj->Nodes.push_back(Nodes[node->Int64Attribute("ref")]);
			node = node->NextSiblingElement("nd");
		}
	}

	//create multipolygon relations
	for (auto current_relation : relations)
	{
		auto id = current_relation->Int64Attribute("id");
		auto node = current_relation->FirstChildElement("member");
		if (!node)
		{
			continue;
		}

		auto relObj = new OsmRelation(current_relation->Int64Attribute("id"));
		std::vector<std::string> roles;

		while (node != nullptr)
		{
			const char* type;
			const char* role;
			if (node->QueryStringAttribute("type", &type) != tinyxml2::XML_SUCCESS)
			{
				continue;
			}
			if (node->QueryStringAttribute("role", &role) != tinyxml2::XML_SUCCESS)
			{
				role = "";
			}

			auto memberId = node->Int64Attribute("ref");

			if (!std::strcmp(type, "node"))
			{
				relObj->NodeRoles[memberId] = role;
			}
			else if (!std::strcmp(type, "way"))
			{
				relObj->WayRoles[memberId] = role;
			}
			else if (!std::strcmp(type, "relation"))
			{
				relObj->RelRoles[memberId] = role;
			}

			node = node->NextSiblingElement("member");
		}

		auto tag = current_relation->FirstChildElement("tag");
		while (tag != nullptr)
		{
			auto key = std::string(tag->Attribute("k"));
			std::transform(key.begin(), key.end(), key.begin(), ::tolower);
			relObj->Tags.Add(key.c_str(), tag->Attribute("v"));
			tag = tag->NextSiblingElement("tag");
		}

		Relations.insert_or_assign(id, relObj);
	}

	//add relation subelements 
	for (auto rel : Relations)
	{
		auto relObj = rel.second;
		for (auto memberNode : relObj->NodeRoles)
		{
			auto iter = Nodes.find(memberNode.first);
			if (iter != Nodes.end())
			{
				relObj->Nodes.insert_or_assign(memberNode.first, (*iter).second);
			}
		}

		for (auto memberWay : relObj->WayRoles)
		{
			auto iter = Ways.find(memberWay.first);
			if (iter != Ways.end())
			{
				relObj->Ways.insert_or_assign(memberWay.first, (*iter).second);
			}
		}

		for (auto memberRelation : relObj->RelRoles)
		{
			auto iter = Relations.find(memberRelation.first);
			if (iter != Relations.end())
			{
				relObj->Relations.insert_or_assign(memberRelation.first, (*iter).second);
			}
		}
	}
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


void UOsmReader::InitBuildingPart(const OsmWay* inWay, FBuildingPart* inPart)
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


void UOsmReader::InitBuildingPart(const OsmRelation* inRelation, FBuildingPart* inPart)
{
	auto floorsTag = FindBuildingTag(inRelation->Tags, "levels");
	inPart->Floors = floorsTag ? FCString::Atoi(**floorsTag) : 1;

	auto heightTag = FindBuildingTag(inRelation->Tags, "height");
	inPart->Height = heightTag
		? FCString::Atoi(**heightTag) * UGeoHelpers::SCALE_MULT
		: inPart->Floors * inPart->FloorHeight + 2 * UGeoHelpers::SCALE_MULT;

	auto minFloorsTag = FindBuildingTag(inRelation->Tags, "min_levels");
	inPart->MinFloors = minFloorsTag ? FCString::Atoi(**minFloorsTag) : 0;

	auto minHeightTag = FindBuildingTag(inRelation->Tags, "min_height");
	inPart->MinHeight = minHeightTag
		? FCString::Atoi(**minHeightTag) * UGeoHelpers::SCALE_MULT
		: inPart->MinFloors * inPart->FloorHeight;

	if (heightTag || minHeightTag)
	{
		inPart->OverrideHeight = true;
	}
}

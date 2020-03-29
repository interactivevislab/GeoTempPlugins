#include "OSMLoader.h"
#include <algorithm>


void UOSMReader::ParseBuildings()
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
	buildings.Empty();
	//find all building and building parts through ways
	for (auto wayP : Ways)
	{
		auto way = wayP.second;
		auto buildIter = way->Tags.Find("building");
		auto partIter = way->Tags.Find("building:part");
		FBuildingPart* part;
		//if this is building or part
		if (buildIter || partIter) {
			//create and init building part data
			part = new FBuildingPart(way->id);

			//parse heights and floor counts
			initBuildingPart(way, part);

			parts.Add(part);
			//if this is building also create building data
			if (buildIter) {
				auto building = new FBuilding(way->id);
				building->Type = TCHAR_TO_UTF8(**buildIter);

				part->Owner = building;
				building->MainPart = part;
				//building->Parts.push_back(part);
				buildings.Add(building);
			}
			else {
				//add this part to list				
				wayParts.insert_or_assign(part->Id, part);
			}

			////get all points of this way and add necessary bindings
			std::vector<FVector> points;

			for (auto node : way->Nodes) {
				points.push_back(node->Point);
				pointsWayParts[node->id].push_back(way->id);
				wayPartPoints[way->id].push_back(node->id);
				
			}
			auto cont = FContour(points);
			part->OuterConts.Add(cont);

			//add part to list
			buildingWayParts.insert_or_assign(way->id, part);
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
			auto building = new FBuilding(relation->id);
			building->Parts.clear();
			building->Type = TCHAR_TO_UTF8(**buildIter);
			buildings.Add(building);

			//create building part data from relation (it will be the footprint)
			part = new FBuildingPart(relation->id);
			initBuildingPart(relation, part);
			building->MainPart = part;

			part->Owner = building;

			//now iterate over the ways in this relation
			for (auto element : relation->WayRoles)
			{
				auto way = relation->Ways[element.first];
				if (!way) continue;
				auto& currentPart = part;
				auto c = new FContour();
				for (auto node : way->Nodes)
				{
					c->points.Add(node->Point);
					pointsRelParts[node->id].push_back(relation->id);
					relPartPoints[relation->id].push_back(node->id);
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
				if (rel->Tags.Find("building:part")) {
					building->Parts.push_back(parts[element.first]);
				}
			}

		}
		//if this relation is building part
		else if (partIter) {
			part = new FBuildingPart(relation->id);
			initBuildingPart(relation, part);
			parts.Add(part);
			relParts.insert_or_assign(part->Id, part);
			for (auto element : relation->WayRoles)
			{
				auto way = relation->Ways[element.first];
				if (!way) continue;
				auto& currentPart = part;
				//if this way is building part, just add it to parts list (we probably have created it already
				if (!way->Tags.Find("building:part"))
				{
					auto c = new FContour();
					for (auto node : way->Nodes)
					{
						c->points.Add(node->Point);
						pointsRelParts[node->id].push_back(relation->id);
						relPartPoints[relation->id].push_back(node->id);
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

void UOSMReader::InitWithXML(FString xmlString)
{
	XMLDocument.Parse(TCHAR_TO_UTF8(*xmlString));
	ReadData();
}

void UOSMReader::InitWithFile(FString filename)
{
	XMLDocument.LoadFile(TCHAR_TO_UTF8(*filename));
	ReadData();
}


void UOSMReader::ReadData()
{
#pragma region Parse Nodes
	std::vector<tinyxml2::XMLElement*> nodes;

	tinyxml2::XMLNode* root = XMLDocument.FirstChild()->NextSibling();

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
			auto nodeObj = new OSMNode(id, lon, lat, OriginLon, OriginLat);
			Nodes.insert_or_assign(id, nodeObj);
		}
		else
		{
			auto nodeObj = new OSMNode(id, lon, lat, OriginLon, OriginLat);
			for (; tag != nullptr; tag = tag->NextSiblingElement("tag"))
			{
				auto key = std::string(tag->Attribute("k"));
				std::transform(key.begin(), key.end(), key.begin(), ::tolower);
				nodeObj->Tags.Add(key.c_str(), tag->Attribute("v"));
			}
			Nodes.insert_or_assign(id, nodeObj);
		}
	}

	//create ways objects
	for (auto currentWay : ways)
	{
		auto node = currentWay->FirstChildElement("nd");
		if (!node) continue;
		auto id = currentWay->Int64Attribute("id");
		auto wayObj = new OSMWay(id);
		auto tag = currentWay->FirstChildElement("tag");
		if (tag != nullptr)
		{
			bool isArea = false;
			for (; tag != nullptr; tag = tag->NextSiblingElement("tag"))
			{
				auto key = std::string(tag->Attribute("k"));
				auto value = std::string(tag->Attribute("v"));
				std::transform(key.begin(), key.end(), key.begin(), ::tolower);
				wayObj->Tags.Add(key.c_str(), tag->Attribute("v"));
			}
		}
		Ways.insert_or_assign(id, wayObj);
		std::vector<OSMNode*> wayNodes;
		for (; node != nullptr; node = node->NextSiblingElement("nd"))
		{
			wayObj->Nodes.push_back(Nodes[node->Int64Attribute("ref")]);
		}
	}

	//create multipolygon relations
	for (auto current_relation : relations)
	{
		auto id = current_relation->Int64Attribute("id");
		auto node = current_relation->FirstChildElement("member");
		if (!node) continue;
		auto relObj = new OSMRelation(current_relation->Int64Attribute("id"));
		std::vector<std::string> roles;
		bool isArea = false;
		for (; node != nullptr; node = node->NextSiblingElement("member"))
		{
			const char* type;
			const char* role;
			if (!node->QueryStringAttribute("type", &type) == tinyxml2::XML_SUCCESS) continue;
			if (!node->QueryStringAttribute("role", &role) == tinyxml2::XML_SUCCESS) role = "";

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
		}

		auto tag = current_relation->FirstChildElement("tag");
		if (tag != nullptr)
		{
			for (; tag != nullptr; tag = tag->NextSiblingElement("tag"))
			{
				auto key = std::string(tag->Attribute("k"));
				std::transform(key.begin(), key.end(), key.begin(), ::tolower);
				relObj->Tags.Add(key.c_str(), tag->Attribute("v"));
			}
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
			if (iter != Nodes.end()) {
				relObj->Nodes.insert_or_assign(memberNode.first, (*iter).second);
			}
		}
		for (auto memberWay : relObj->WayRoles)
		{
			auto iter = Ways.find(memberWay.first);
			if (iter != Ways.end()) {
				relObj->Ways.insert_or_assign(memberWay.first, (*iter).second);
			}
		}
		for (auto memberRelation : relObj->RelRoles)
		{
			auto iter = Relations.find(memberRelation.first);
			if (iter != Relations.end()) {
				relObj->Relations.insert_or_assign(memberRelation.first, (*iter).second);
			}
		}
	}
}

void UOSMReader::initBuildingPart(const OSMWay* way, FBuildingPart* part)
{
	auto floorsIt = way->Tags.Find("building:levels");
	if (!floorsIt) floorsIt = way->Tags.Find("levels");
	part->Floors = floorsIt ? FCString::Atoi(**floorsIt) : 1;

	auto heightIt = way->Tags.Find("building:height");
	if (!heightIt) heightIt = way->Tags.Find("height");
	part->Height = heightIt ? FCString::Atoi(**heightIt) * scaleMult : part->Floors * part->floorHeight + 2 * scaleMult;

	auto mfloorsIt = way->Tags.Find("building:min_levels");
	if (!mfloorsIt) mfloorsIt = way->Tags.Find("min_levels");
	part->MinFloors = mfloorsIt ? FCString::Atoi(**mfloorsIt) : 0;

	auto mheightIt = way->Tags.Find("building:min_height");
	if (!mheightIt) mheightIt = way->Tags.Find("min_height");
	part->MinHeight = mheightIt ? FCString::Atoi(**mheightIt) * scaleMult : part->MinFloors * part->floorHeight;

	if (heightIt || mheightIt) part->OverrideHeight = true;
}

void UOSMReader::initBuildingPart(const OSMRelation* relation, FBuildingPart* part)
{
	auto floorsIt = relation->Tags.Find("building:levels");
	if (!floorsIt) floorsIt = relation->Tags.Find("levels");
	part->Floors = floorsIt ? FCString::Atoi(**floorsIt) : 1;

	auto heightIt = relation->Tags.Find("building:height");
	if (!heightIt) heightIt = relation->Tags.Find("height");
	part->Height = heightIt ? FCString::Atoi(**heightIt) * scaleMult : part->Floors * part->floorHeight + 2 * scaleMult;

	auto mfloorsIt = relation->Tags.Find("building:min_levels");
	if (!mfloorsIt) mfloorsIt = relation->Tags.Find("min_levels");
	part->MinFloors = mfloorsIt ? FCString::Atoi(**mfloorsIt) : 0;

	auto mheightIt = relation->Tags.Find("building:min_height");
	if (!mheightIt) mheightIt = relation->Tags.Find("min_height");
	part->MinHeight = mheightIt ? FCString::Atoi(**mheightIt) * scaleMult : part->MinFloors * part->floorHeight;

	if (heightIt || mheightIt) part->OverrideHeight = true;
}
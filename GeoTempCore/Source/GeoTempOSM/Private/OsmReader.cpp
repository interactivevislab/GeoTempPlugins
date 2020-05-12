#include "OsmReader.h"
#include <algorithm>


UOsmReader::UOsmReader() : UObject()
{
}

void UOsmReader::ClearReaderData()
{
	Nodes.Empty();
	Ways.Empty();
	Relations.Empty();
}

void UOsmReader::InitWithXML(FString inXmlString)
{	
	xmlDocument.Parse(TCHAR_TO_UTF8(*inXmlString));
	if (xmlDocument.FirstChild()) {
		ReadData();
	}
}


void UOsmReader::InitWithFile(FString inFilename)
{
	xmlDocument.LoadFile(TCHAR_TO_UTF8(*inFilename));
	if (xmlDocument.FirstChild()) {
		ReadData();
	}
}


void UOsmReader::ReadData()
{
#pragma region Parse Nodes
	std::vector<tinyxml2::XMLElement*> nodes;

	tinyxml2::XMLNode* root = xmlDocument.FirstChild()->NextSibling();

	tinyxml2::XMLElement* bounds = root->FirstChildElement("bounds");

	if (bounds)
	{
		double minY = bounds->DoubleAttribute("minlat");
		double maxY = bounds->DoubleAttribute("maxlat");

		double minX = bounds->DoubleAttribute("minlon");
		double maxX = bounds->DoubleAttribute("maxlon");

		auto topLeftCorner = UGeoHelpers::GetLocalCoordinates(minX, maxY, 0, GeoCoords);
		auto bottomRightCorner = UGeoHelpers::GetLocalCoordinates(maxX, minY, 0, GeoCoords);

		BoundsRect = FVector4(topLeftCorner.X, bottomRightCorner.X, topLeftCorner.Y, bottomRightCorner.Y);
	}

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
			Nodes.Add(id, nodeObj);
		}
		else
		{
			auto nodeObj = new OsmNode(id, lon, lat, GeoCoords);
			while (tag != nullptr)
			{
				auto key = std::string(tag->Attribute("k"));
				auto value = std::string(tag->Attribute("v"));
				std::transform(key.begin(), key.end(), key.begin(), ::tolower);

				auto keyString = FString(UTF8_TO_TCHAR(key.c_str()));
				auto valueString = FString(UTF8_TO_TCHAR(value.c_str()));
				nodeObj->Tags.Add(keyString, valueString);
				tag = tag->NextSiblingElement("tag");
			}
			Nodes.Add(id, nodeObj);
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

			auto keyString = FString(UTF8_TO_TCHAR(key.c_str()));
			auto valueString = FString(UTF8_TO_TCHAR(value.c_str()));
			wayObj->Tags.Add(keyString, valueString);
			tag = tag->NextSiblingElement("tag");
		}

		Ways.Add(id, wayObj);

		while (node != nullptr)
		{
			wayObj->Nodes.Add(Nodes[node->Int64Attribute("ref")]);
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
				relObj->NodeRoles.Add(memberId, role);
			}
			else if (!std::strcmp(type, "way"))
			{
				relObj->WayRoles.Add(memberId, role);
			}
			else if (!std::strcmp(type, "relation"))
			{
				relObj->RelRoles.Add(memberId, role);
			}

			node = node->NextSiblingElement("member");
		}

		auto tag = current_relation->FirstChildElement("tag");
		while (tag != nullptr)
		{
			auto key = std::string(tag->Attribute("k"));
			auto value = std::string(tag->Attribute("v"));
			std::transform(key.begin(), key.end(), key.begin(), ::tolower);

			auto keyString = FString(UTF8_TO_TCHAR(key.c_str()));
			auto valueString = FString(UTF8_TO_TCHAR(value.c_str()));
			relObj->Tags.Add(keyString, valueString);
			tag = tag->NextSiblingElement("tag");
		}

		Relations.Add(id, relObj);
	}

	//add relation subelements 
	for (auto rel : Relations)
	{
		auto relObj = rel.Value;
		for (auto memberNode : relObj->NodeRoles)
		{
			auto iter = Nodes.Find(memberNode.Key);
			if (iter != nullptr)
			{
				relObj->Nodes.Add(memberNode.Key, *iter);
			}
		}

		for (auto memberWay : relObj->WayRoles)
		{
			auto iter = Ways.Find(memberWay.Key);
			if (iter != nullptr)
			{
				relObj->Ways.Add(memberWay.Key, *iter);
			}
		}

		for (auto memberRelation : relObj->RelRoles)
		{
			auto iter = Relations.Find(memberRelation.Key);
			if (iter != nullptr)
			{
				relObj->Relations.Add(memberRelation.Key, *iter);
			}
		}
	}
}




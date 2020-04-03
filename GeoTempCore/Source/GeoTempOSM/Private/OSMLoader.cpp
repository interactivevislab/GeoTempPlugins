#include "OSMLoader.h"
#include <algorithm>


UOsmReader::UOsmReader() : UObject()
{
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




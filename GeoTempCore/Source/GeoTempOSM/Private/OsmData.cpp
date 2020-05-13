#include "OsmData.h"


OsmNode::OsmNode(long id, double lon, double lat, FGeoCoords geoCoords, float height) : Id(id)
{
    Point = UGeoHelpers::GetLocalCoordinates(lon, lat, 0, geoCoords);
}


OsmWay::OsmWay(long id) : Id(id)
{
}


OsmRelation::OsmRelation(long id) : Id(id)
{
}

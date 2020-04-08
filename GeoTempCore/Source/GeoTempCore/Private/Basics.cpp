#include "Basics.h"

#include "triangle/triangulate.h"

#include <unordered_set>


struct Edge
{
	int t1;
	int t2;
	bool operator==(const Edge& v) const
	{
		return	(t1 == v.t1 && t2 == v.t2) ||
				(t1 == v.t2 && t2 == v.t1);
	}
};


namespace std
{
	template <>
	struct hash<Edge>
	{
		size_t operator()(const Edge& k) const
		{
			// Compute individual hash values for two data members and combine them using XOR and bit shifting
			return (hash<int>()(k.t1) ^ hash<int>()(k.t2));
		}
	};
}


void Triangulate(TArray<FContour>& outOuter, TArray<FContour>& outInner, TArray<FVector>& outPoints,
	TArray<int>& outTriangles, std::string inFlags, TArray<FContour> inOtherLines, int& outContourPointsNum)
{
	Eigen::MatrixXd V;
	Eigen::MatrixXi E;
	Eigen::MatrixXd H;

	Eigen::MatrixXd V2;
	Eigen::MatrixXi F2;

	std::vector<float> inPoints;
	std::vector<int> inEdges;
	std::vector<float> holes;
	outPoints.Empty();
	outTriangles.Empty();
	int c_nodes = 0;
	std::unordered_set<Edge> addEdges;
	TMap<FVector, int> nodeIds;
	int nodeId = 0;
	int prevId = -1;
	for (auto& way : outOuter)
	{
		int firstId = -1;
		for (int i = 0; i < way.Points.Num(); i++)
		{
			auto node = way.Points[i];

			if (!nodeIds.Contains(node))
			{
				outPoints.Add(node);
				nodeIds.Add(node, nodeId++);
				inPoints.push_back(node.X);
				inPoints.push_back(node.Y);
			}

			int id = nodeIds[node];
			if (i == 0) firstId = id;

			if ((i > 0) && (prevId != id) && (addEdges.find(Edge{ prevId, id }) == addEdges.end()))
			{
				inEdges.push_back(prevId);
				inEdges.push_back(id);
				addEdges.insert(Edge{ prevId, id });
			}
			prevId = id;
		}

		if ((prevId != firstId) && (addEdges.find(Edge{ prevId, firstId }) == addEdges.end()))
		{
			inEdges.push_back(prevId);
			inEdges.push_back(firstId);
			addEdges.insert(Edge{ prevId, firstId });
		}

		c_nodes = outPoints.Num();
	}

	for (auto& way : outInner)
	{
		int firstId = -1;
		for (int i = 0; i < way.Points.Num(); i++)
		{
			auto node = way.Points[i];
			if (i == 0 || !(node - way.Points[0]).IsNearlyZero())
			{
				outPoints.Add(node);

				if (!nodeIds.Contains(node))
				{
					nodeIds.Add(node, nodeId++);
					inPoints.push_back(node.X);
					inPoints.push_back(node.Y);
				}

				int id = nodeIds[node];
				if (i == 0) firstId = id;

				if ((i > 0) && (prevId != id) && (addEdges.find(Edge{ prevId, id }) == addEdges.end()))
				{
					inEdges.push_back(prevId);
					inEdges.push_back(id);
					addEdges.insert(Edge{ prevId, id });
				}
				prevId = id;
			}
			else if ((i != 0) && (prevId != firstId) && (addEdges.find(Edge{ prevId, firstId }) == addEdges.end()))
			{
				inEdges.push_back(prevId);
				inEdges.push_back(firstId);
				addEdges.insert(Edge{ prevId, firstId });
			}
		}

		//add hole
		int ind = way.LeftmostIndex();
		int indMinus	= (ind - 1 + way.Points.Num()) % way.Points.Num();
		int indPlus		= (ind + 1 + way.Points.Num()) % way.Points.Num();
		while (way.Points[indMinus].Equals(way.Points[ind]) && indMinus != ind)
		{
			indMinus = (indMinus - 1 + way.Points.Num()) % way.Points.Num();
		}
		while (way.Points[indPlus].Equals(way.Points[ind]) && indPlus != ind)
		{
			indPlus = (indPlus + 1 + way.Points.Num()) % way.Points.Num();
		}

		auto delta = (way.Points[indMinus] + way.Points[indPlus] - way.Points[ind] * 2).GetSafeNormal2D() * 10.0f;
		auto p = way.Points[ind] + delta;
		holes.push_back(p.X);
		holes.push_back(p.Y);
		c_nodes = outPoints.Num();
	}
	outContourPointsNum = c_nodes;
	for (auto& way : inOtherLines)
	{
		int firstId = -1;
		for (int i = 0; i < way.Points.Num(); i++)
		{
			auto node = way.Points[i];

			if (!nodeIds.Contains(node))
			{
				outPoints.Add(node);
				nodeIds.Add(node, nodeId++);
				inPoints.push_back(node.X);
				inPoints.push_back(node.Y);
			}

			int id = nodeIds[node];
			if (i == 0)
			{
				firstId = id;
			}

			if ((i > 0) && (prevId != id) && (addEdges.find(Edge{ prevId, id }) == addEdges.end()))
			{
				inEdges.push_back(prevId);
				inEdges.push_back(id);
				addEdges.insert(Edge{ prevId, id });
			}
			prevId = id;
		}
		c_nodes = outPoints.Num();
	}

	if (inPoints.size() == 0) return;
	V.resize(inPoints.size() / 2, 2);
	E.resize(inEdges.size() / 2, 2);
	H.resize(holes.size() / 2, 2);

	bool isX = true;
	for (int i = 0; i < inPoints.size(); i += 2)
	{
		V(i / 2, 0) = inPoints[i];
		V(i / 2, 1) = inPoints[i + 1];
	}
	for (int i = 0; i < inEdges.size(); i += 2)
	{
		E(i / 2, 0) = inEdges[i];
		E(i / 2, 1) = inEdges[i + 1];
	}
	for (int i = 0; i < holes.size(); i += 2)
	{
		H(i / 2, 0) = holes[i];
		H(i / 2, 1) = holes[i + 1];
	}

	if (inPoints.size() < 6) return;

	igl::triangle::triangulate(V, E, H, inFlags, V2, F2);

	outPoints.Empty();

	for (int i = 0; i < V2.rows(); i++)
	{
		outPoints.Add(FVector(V2(i, 0), V2(i, 1), 0));
	}

	for (int i = 0; i < F2.rows(); i++)
	{
		outTriangles.Add(F2(i, 0));
		outTriangles.Add(F2(i, 2));
		outTriangles.Add(F2(i, 1));
	}
}

void Triangulate(TArray<FContour>& outOuter, TArray<FContour>& outInner, TArray<FVector>& outPoints,
	TArray<int>& outTriangles, std::string inFlags)
{
	int t;
	return Triangulate(outOuter, outInner, outPoints, outTriangles, inFlags, TArray<FContour>(), t);
}


FGeoCoords::FGeoCoords() {};


FGeoCoords::FGeoCoords(ProjectionType projection, float zeroLon, float zeroLat) : Projection(projection), 
	ZeroLon(zeroLon), ZeroLat(zeroLat) {}


#pragma region UGeoHelpers

const double UGeoHelpers::EARTH_RADIUS = 6378137;
const double UGeoHelpers::SCALE_MULT = 100;


inline double UGeoHelpers::DegreesToRadians(double inAngle)
{
	return inAngle * PI / 180;
}


inline double UGeoHelpers::RadiansToDegrees(double inAngle)
{
	return inAngle * 180 / PI;
}


FVector UGeoHelpers::GetLocalCoordinates(double inX, double inY, double inZ, FGeoCoords inGeoCoords)
{
	switch (inGeoCoords.Projection)
	{
		case ProjectionType::LOCAL_METERS:
			return FVector(inX, inY, inZ) * SCALE_MULT;
			break;

		case ProjectionType::WGS84_PsevdoMerkator:
		{
			double ox = DegreesToRadians(inGeoCoords.ZeroLon) * EARTH_RADIUS;
			double oy = log(tan(DegreesToRadians(inGeoCoords.ZeroLat) / 2 + PI / 4)) * EARTH_RADIUS;
			return (FVector(float((inX - ox) * SCALE_MULT), float((inY - oy) * SCALE_MULT), inZ * SCALE_MULT));
		}

		case ProjectionType::WGS84:
		{
			double ox = DegreesToRadians(inGeoCoords.ZeroLon) * EARTH_RADIUS;
			double oy = log(tan(DegreesToRadians(inGeoCoords.ZeroLat) / 2 + PI / 4)) * EARTH_RADIUS;
			double s = cos(DegreesToRadians(inGeoCoords.ZeroLat));
			double x1 = DegreesToRadians(inX) * EARTH_RADIUS;
			double y1 = log(tan(DegreesToRadians(inY) / 2 + PI / 4)) * EARTH_RADIUS;

			return -FVector(float((ox + -x1) * SCALE_MULT * s), float((y1 - oy) * SCALE_MULT * s), inZ * SCALE_MULT);
		}
	}

	return FVector::ZeroVector;
}


FVector2D UGeoHelpers::ConvertToLonLat(float inX, float inY, FGeoCoords inGeoCoords)
{
	switch (inGeoCoords.Projection)
	{
		case ProjectionType::LOCAL_METERS:
			UE_LOG(LogTemp, Warning, TEXT("Attempt to get geocoordinates with local projection"));
			return FVector2D::ZeroVector;

		case ProjectionType::WGS84_PsevdoMerkator:
		{
			double ox = DegreesToRadians(inGeoCoords.ZeroLon) * EARTH_RADIUS;
			double oy = log(tan(DegreesToRadians(inGeoCoords.ZeroLat) / 2 + PI / 4)) * EARTH_RADIUS;

			double posX = ox + inX / SCALE_MULT;
			double posY = oy + inY / SCALE_MULT;

			return FVector2D(posX, posY);
		}

		case ProjectionType::WGS84:
		{
			double ox = DegreesToRadians(inGeoCoords.ZeroLon) * EARTH_RADIUS;
			double oy = log(tan(DegreesToRadians(inGeoCoords.ZeroLat) / 2 + PI / 4)) * EARTH_RADIUS;

			double s = cos(DegreesToRadians(inGeoCoords.ZeroLat));
			double posX1 = inX / SCALE_MULT / s + ox;
			double posY1 = -inY / SCALE_MULT / s + oy;

			double posX = RadiansToDegrees(posX1 / EARTH_RADIUS);
			double posY = RadiansToDegrees(2 * atan(exp(posY1 / EARTH_RADIUS)) - PI / 2);
			return FVector2D(posX, posY);
		}
	}
	return FVector2D::ZeroVector;
}

#pragma endregion

// Fill out your copyright notice in the Description page of Project Settings.


#include "Basics.h"
#include "triangle/triangulate.h"
#include <unordered_set>

struct Edge
{
	int t1;
	int t2;
	bool operator==(const Edge& v) const
	{
		return (t1 == v.t1 && t2 == v.t2) || (t1 == v.t2 && t2 == v.t1);
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

void Triangulate(TArray<FContour>& outer, TArray<FContour>& inner, std::vector<FVector>& points, std::vector<int>& triangles, std::string flags, TArray<FContour> otherLines, int& contourPointsNum)
{
	Eigen::MatrixXd V;
	Eigen::MatrixXi E;
	Eigen::MatrixXd H;

	Eigen::MatrixXd V2;
	Eigen::MatrixXi F2;

	std::vector<float> inPoints;
	std::vector<int> inEdges;
	std::vector<float> holes;
	points.clear();
	triangles.clear();
	int c_nodes = 0;
	std::unordered_set<Edge> addEdges;
	TMap<FVector, int> nodeIds;
	int nodeId = 0;
	int prevId = -1;
	for (auto& way : outer)
	{
		int firstId = -1;
		for (int i = 0; i < way.points.Num(); i++) {
			auto node = way.points[i];

			if (!nodeIds.Contains(node)) {
				points.push_back(node);
				nodeIds.Add(node, nodeId++);
				inPoints.push_back(node.X);
				inPoints.push_back(node.Y);
			}
			int id = nodeIds[node];
			if (i == 0) firstId = id;

			if (i > 0) {
				if (prevId != id) {
					if (addEdges.find(Edge{ prevId, id }) == addEdges.end()) {
						inEdges.push_back(prevId);
						inEdges.push_back(id);
						addEdges.insert(Edge{ prevId, id });
					}
				}
			}
			prevId = id;
		}
		if (prevId != firstId) {
			if (addEdges.find(Edge{ prevId, firstId }) == addEdges.end()) {
				inEdges.push_back(prevId);
				inEdges.push_back(firstId);
				addEdges.insert(Edge{ prevId, firstId });
			}
		}
		c_nodes = points.size();
	}
	for (auto& way : inner)
	{
		int firstId = -1;
		for (int i = 0; i < way.points.Num(); i++) {
			auto node = way.points[i];
			if (i == 0 || !(node - way.points[0]).IsNearlyZero()) {
				points.push_back(node);

				if (!nodeIds.Contains(node)) {
					nodeIds.Add(node, nodeId++);
					inPoints.push_back(node.X);
					inPoints.push_back(node.Y);

				}
				int id = nodeIds[node];
				if (i == 0) firstId = id;

				if (i > 0) {
					if (prevId != id) {
						if (addEdges.find(Edge{ prevId, id }) == addEdges.end()) {
							inEdges.push_back(prevId);
							inEdges.push_back(id);
							addEdges.insert(Edge{ prevId, id });
						}
					}
				}
				prevId = id;
			}
			else if (i != 0) {
				if (prevId != firstId) {
					if (addEdges.find(Edge{ prevId, firstId }) == addEdges.end()) {
						inEdges.push_back(prevId);
						inEdges.push_back(firstId);
						addEdges.insert(Edge{ prevId, firstId });
					}
				}
			}
		}
		//add hole
		int ind = way.LeftmostIndex();
		int indMinus = (ind - 1 + way.points.Num()) % way.points.Num();
		int indPlus = (ind + 1 + way.points.Num()) % way.points.Num();
		while (way.points[indMinus].Equals(way.points[ind]) && indMinus != ind) indMinus = (indMinus - 1 + way.points.Num()) % way.points.Num();
		while (way.points[indPlus].Equals(way.points[ind]) && indPlus != ind) indPlus = (indPlus + 1 + way.points.Num()) % way.points.Num();
		auto delta = (way.points[indMinus] + way.points[indPlus] - way.points[ind] * 2).GetSafeNormal2D() * 10.0f;
		auto p = way.points[ind] + delta;
		holes.push_back(p.X);
		holes.push_back(p.Y);
		c_nodes = points.size();
	}
	contourPointsNum = c_nodes;
	for (auto& way : otherLines)
	{
		int firstId = -1;
		for (int i = 0; i < way.points.Num(); i++) {
			auto node = way.points[i];

			if (!nodeIds.Contains(node)) {
				points.push_back(node);
				nodeIds.Add(node, nodeId++);
				inPoints.push_back(node.X);
				inPoints.push_back(node.Y);
			}
			int id = nodeIds[node];
			if (i == 0) firstId = id;

			if (i > 0) {
				if (prevId != id) {
					if (addEdges.find(Edge{ prevId, id }) == addEdges.end()) {
						inEdges.push_back(prevId);
						inEdges.push_back(id);
						addEdges.insert(Edge{ prevId, id });
					}
				}
			}
			prevId = id;
		}
		c_nodes = points.size();
	}

	if (inPoints.size() == 0) return;
	V.resize(inPoints.size() / 2, 2);
	E.resize(inEdges.size() / 2, 2);
	H.resize(holes.size() / 2, 2);

	bool isX = true;
	for (int i = 0; i < inPoints.size(); i += 2) {
		V(i / 2, 0) = inPoints[i];
		V(i / 2, 1) = inPoints[i + 1];
	}
	for (int i = 0; i < inEdges.size(); i += 2) {
		E(i / 2, 0) = inEdges[i];
		E(i / 2, 1) = inEdges[i + 1];
	}
	for (int i = 0; i < holes.size(); i += 2) {
		H(i / 2, 0) = holes[i];
		H(i / 2, 1) = holes[i + 1];
	}

	if (inPoints.size() < 6) return;
	// Triangulate the interior
	igl::triangle::triangulate(V, E, H, flags, V2, F2);

	points.clear();

	for (int i = 0; i < V2.rows(); i++)
	{
		points.push_back(FVector(V2(i, 0), V2(i, 1), 0));
	}

	for (int i = 0; i < F2.rows(); i++)
	{
		triangles.push_back(F2(i, 0));
		triangles.push_back(F2(i, 2));
		triangles.push_back(F2(i, 1));
	}
}

void Triangulate(TArray<FContour>& outer, TArray<FContour>& inner, std::vector<FVector>& points, std::vector<int>& triangles, std::string flags)
{
	int t;
	return Triangulate(outer, inner, points, triangles, flags, TArray<FContour>(), t);
}
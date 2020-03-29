#pragma once

#include <vector>
#include <unordered_map>
#include <CoreMinimal.h>
#include <limits>	


#define scaleMult 100
namespace BasicData
{
#ifndef PI
#define PI (3.1415926535897932)
#endif
#define DEG2RAD(a) ((a) / (180 / PI))
#define RAD2DEG(a) ((a) * (180 / PI))
#define EARTH_RADIUS 6378137


	

	struct Curve
	{
		std::vector<FVector> Points;

		Curve() {}

		Curve(std::vector<FVector> points)
		{
			for (auto point : points)
			{
				Points.push_back(point);
			}
			Points.reserve(points.size());
		}

		bool IsClockwise() {
			int iMin = LeftmostIndex();
			int iMinus = (iMin - 1 + Points.size()) % Points.size();
			int iPlus = (iMin + 1) % Points.size();

			auto& a = Points[iMin];
			auto& b = Points[iMinus];
			auto& c = Points[iPlus];

			return (b.X - a.X) * (c.Y - a.Y) - (b.Y - a.Y) * (c.X - a.X) > 0;
		}

		int LeftmostIndex() {
			int minInd = 0;
			float minX = Points[0].X;
			for (int i = 1; i < Points.size(); i++) {
				if (Points[i].X < minX) {
					minInd = i;
					minX = Points[i].X;
				}
			}
			return minInd;
		}

		Curve* ClearContour() {
			std::vector<FVector> newPoints;
			for (int i = 0; i < Points.size(); i++) {
				int i0 = (i - 1 + Points.size()) % Points.size();
				if (!(Points[i] == Points[i0])) newPoints.push_back(Points[i]);
			}
			return new Curve(newPoints);
		}

		Curve* Revert() {
			std::vector<FVector> newPoints;
			for (int i = Points.size() - 1; i >= 0; i--) {
				newPoints.push_back(Points[i]);
			}
			return new Curve(newPoints);
		}
	};

	struct Multypoligon
	{
		std::vector<Curve*> Outer;

		std::vector<Curve*> Inner;
	};

}
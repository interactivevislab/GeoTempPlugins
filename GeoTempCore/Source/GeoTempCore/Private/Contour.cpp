#include "Contour.h"


FContour::FContour() {}


FContour::FContour(std::vector<FVector> initPoints)
{
	for (auto point : initPoints)
	{
		Points.Add(point);
	}
}


FContour::FContour(TArray<FVector> initPoints)
{
	for (auto point : initPoints)
	{
		Points.Add(point);
	}
}


int FContour::LeftmostIndex() const
{
	int minInd = 0;
	float minX = Points[0].X;
	for (int i = 1; i < Points.Num(); i++)
	{
		if (Points[i].X < minX)
		{
			minInd = i;
			minX = Points[i].X;
		}
	}
	return minInd;
}


int FContour::RightmostIndex() const
{
	int minInd = 0;
	float maxX = Points[0].X;
	for (int i = 1; i < Points.Num(); i++)
	{
		if (Points[i].X > maxX)
		{
			minInd = i;
			maxX = Points[i].X;
		}
	}
	return minInd;
}


int FContour::TopmostIndex() const
{
	int minInd = 0;
	float minY = Points[0].Y;
	for (int i = 1; i < Points.Num(); i++)
	{
		if (Points[i].Y < minY)
		{
			minInd = i;
			minY = Points[i].Y;
		}
	}
	return minInd;
}


int FContour::BottommostIndex() const
{
	int minInd = 0;
	float maxY = Points[0].Y;
	for (int i = 1; i < Points.Num(); i++)
	{
		if (Points[i].Y > maxY)
		{
			minInd = i;
			maxY = Points[i].Y;
		}
	}
	return minInd;
}


bool FContour::FixClockwise(bool inReverse)
{
	if ((Points.Last() - Points[0]).Size2D() > 1)
	{
		auto v = Points[0];
		Points.Add(v);
	}

	int i = LeftmostIndex();
	int i1 = (i + 1) % Points.Num();
	int i0 = (i - 1 + Points.Num()) % Points.Num();

	if ((Points[i1] - Points[i]).Size2D() < 1)
	{
		i1 = (i1 + 1) % Points.Num();
	}
	if ((Points[i0] - Points[i]).Size2D() < 1)
	{
		i0 = (i0 - 1 + Points.Num()) % Points.Num();
	}

	bool needReverse = FVector::CrossProduct(Points[i] - Points[i0], Points[i1] - Points[i]).Z * (inReverse ? -1 : 1) < 0;
	if (needReverse)
	{
		Algo::Reverse(Points);
	}
	return needReverse;
}


void FContour::Cleanup()
{
	TArray<FVector> goodPoints;

	for (int i = 0; i < Points.Num(); i++)
	{
		auto point1 = Points[i];
		auto point2 = Points[(i + 1) % Points.Num()];
		if ((point1 - point2).Size2D() >= 5)
		{
			goodPoints.Add(point1);
		}
	}

	Points = goodPoints;
}


FContour FContour::RemoveCollinear(FContour inContour, float inThreshold)
{
	TArray<FVector> pointsNew;
	bool revert = inContour.FixClockwise();
	int i0 = inContour.LeftmostIndex();
	int i1 = (i0 + 1) % inContour.Points.Num();
	pointsNew.Add(inContour.Points[i0]);
	pointsNew.Add(inContour.Points[i1]);

	for (int i = (i1 + 1) % inContour.Points.Num(); i != i0; i = (i + 1) % inContour.Points.Num())
	{
		auto flag = true;
		while (flag && pointsNew.Num() > 1)
		{
			auto p0 = pointsNew[pointsNew.Num() - 2];
			auto p1 = pointsNew[pointsNew.Num() - 1];
			auto dir0 = p1 - p0;
			auto dir1 = inContour.Points[i] - p1;

			float turn = FMath::Abs(FVector::CrossProduct(dir0.GetSafeNormal(), dir1.GetSafeNormal()).Z);
			if (turn < inThreshold)
			{
				pointsNew.RemoveAt(pointsNew.Num() - 1);
			}
			else
			{
				flag = false;
			}
		}
		pointsNew.Add(inContour.Points[i]);
	}
	FContour cont;
	cont.Points = pointsNew;
	if (revert) cont.FixClockwise(true);
	return cont;
}


FContour FContour::MakeConvex(FContour inContour)
{
	TArray<FVector> pointsNew;
	bool revert = inContour.FixClockwise();
	int i0 = inContour.LeftmostIndex();
	int i1 = (i0 + 1) % inContour.Points.Num();
	pointsNew.Add(inContour.Points[i0]);
	pointsNew.Add(inContour.Points[i1]);

	for (int i = (i1 + 1) % inContour.Points.Num(); i != i0; i = (i + 1) % inContour.Points.Num())
	{
		auto flag = true;
		while (flag && pointsNew.Num() > 1)
		{
			auto p0 = pointsNew[pointsNew.Num() - 2];
			auto p1 = pointsNew[pointsNew.Num() - 1];
			auto dir0 = p1 - p0;
			auto dir1 = inContour.Points[i] - p1;

			if (FVector::CrossProduct(dir0, dir1).Z < 0)
			{
				pointsNew.RemoveAt(pointsNew.Num() - 1);
			}
			else
			{
				flag = false;
			}
		}
		pointsNew.Add(inContour.Points[i]);
	}
	FContour cont;
	cont.Points = pointsNew;
	if (revert)
	{
		cont.FixClockwise(true);
	}
	return cont;
}


inline FContour FContour::RemoveCollinear(float inThreshold) const
{
	return RemoveCollinear(*this, inThreshold);
}


inline FContour FContour::MakeConvex() const
{
	return MakeConvex(*this);
}

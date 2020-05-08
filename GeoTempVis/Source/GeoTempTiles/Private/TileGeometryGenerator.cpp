#include "TileGeometryGenerator.h"

void UTileGeometryGenerator::SetHeightCalculator(TScriptInterface<IHeightCalculator> inCalculator)
{
	HeightCalculator = inCalculator;
}

void UTileGeometryGenerator::GenerateVertices(UTileData* inTile, FString inChannel, int inResolution,
                                              TArray<FVector>& inoutVertices, TArray<FVector>& outNormals)
{
	auto right = inTile->GetRightNeighbor();
	auto bottom = inTile->GetBottomNeighbor();
	auto bottomRight = inTile->GetBottomRightNeighbor();

	int h = (int)inTile->Textures[inChannel]->GetSurfaceHeight();
	int w = (int)inTile->Textures[inChannel]->GetSurfaceWidth();


	auto& tileData = inTile->HeightMap;
	auto& rightData = right->HeightMap;
	auto& bottomData = bottom->HeightMap;
	auto& brData = bottomRight->HeightMap;

	auto delta = inoutVertices[2 + inResolution] - inoutVertices[0];

	auto object = HeightCalculator.GetObject();
	auto interface = Cast<IHeightCalculator>(object);
	if (!IsValid(object)) return;
	if (inResolution < 1) inResolution = 1;
	for (int ix = 0; ix < inResolution + 1; ix++)
	{
		for (int iy = 0; iy < inResolution + 1; iy++)
		{
			int xNorm = (ix / inResolution);
			int yNorm = (iy / inResolution);
			if (ix < inResolution && iy < inResolution)
			{
				inoutVertices[ix * (inResolution + 1) + iy].Z = interface->Execute_CalcHeight(
					object, tileData[((w - 1) * ix / inResolution) + w * ((h - 1) * iy / inResolution)]);
			}
			else if (ix < inResolution)
			{
				inoutVertices[ix * (inResolution + 1) + iy].Z = interface->Execute_CalcHeight(
					object, bottomData[((w - 1) * ix / inResolution) + 0]);
			}
			else if (iy < inResolution)
			{
				inoutVertices[ix * (inResolution + 1) + iy].Z = interface->Execute_CalcHeight(
					object, rightData[0 + w * ((h - 1) * iy / inResolution)]);
			}
			else
			{
				inoutVertices[ix * (inResolution + 1) + iy].Z = interface->Execute_CalcHeight(object, brData[0]);
			}
		}
	}
	for (int ix = 0; ix < inResolution + 1; ix++)
	{
		for (int iy = 0; iy < inResolution + 1; iy++)
		{
			if (ix < inResolution - 1 && iy < inResolution - 1)
			{
				FVector r = inoutVertices[(ix + 1) * (inResolution + 1) + iy] - inoutVertices[ix * (inResolution + 1) + iy];
				FVector b = inoutVertices[ix * (inResolution + 1) + iy + 1] - inoutVertices[ix * (inResolution + 1) + iy];
				outNormals[ix * (inResolution + 1) + iy] = FVector::CrossProduct(r, b).GetSafeNormal();
			}
			else if (ix < inResolution - 1)
			{
				float hr = Cast<IHeightCalculator>(HeightCalculator.GetObject())->Execute_CalcHeight(
					HeightCalculator.GetObject(),
					bottomData[((w - 1) * ix / inResolution) + w * ((h - 1) * 1 / inResolution)]);
				FVector r = inoutVertices[(ix + 1) * (inResolution + 1) + iy] - inoutVertices[ix * (inResolution + 1) + iy];
				FVector b = FVector(0, delta.Y, hr - inoutVertices[ix * (inResolution + 1) + iy].Z);
				outNormals[ix * (inResolution + 1) + iy] = FVector::CrossProduct(r, b).GetSafeNormal();
			}
			else if (iy < inResolution - 1)
			{
				float hb = Cast<IHeightCalculator>(HeightCalculator.GetObject())->Execute_CalcHeight(
					HeightCalculator.GetObject(),
					rightData[((w - 1) * 1 / inResolution) + w * ((h - 1) * iy / inResolution)]);
				FVector r = FVector(delta.X, 0, hb - inoutVertices[ix * (inResolution + 1) + iy].Z);
				FVector b = inoutVertices[ix * (inResolution + 1) + iy + 1] - inoutVertices[ix * (inResolution + 1) + iy];
				outNormals[ix * (inResolution + 1) + iy] = FVector::CrossProduct(r, b).GetSafeNormal();
			}
			else
			{
				float hr = Cast<IHeightCalculator>(HeightCalculator.GetObject())->Execute_CalcHeight(
					HeightCalculator.GetObject(),
					bottomData[((w - 1) * ix / inResolution) + w * ((h - 1) * 1 / inResolution)]);
				float hb = Cast<IHeightCalculator>(HeightCalculator.GetObject())->Execute_CalcHeight(
					HeightCalculator.GetObject(),
					rightData[((w - 1) * 1 / inResolution) + w * ((h - 1) * iy / inResolution)]);
				FVector r = FVector(delta.X, 0, hb - inoutVertices[ix * (inResolution + 1) + iy].Z);
				FVector b = FVector(0, delta.Y, hr - inoutVertices[ix * (inResolution + 1) + iy].Z);
				outNormals[ix * (inResolution + 1) + iy] = FVector::CrossProduct(r, b).GetSafeNormal();
			}
		}
	}
}

#include "TileGeometryGenerator.h"
#include "Materials/MaterialInstanceDynamic.h"

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
    if (inResolution > h) inResolution = h;
    if (inResolution > w) inResolution = w;
    for (int ix = 0; ix < inResolution + 1; ix++)
    {
        for (int iy = 0; iy < inResolution + 1; iy++)
        {
            int xNorm = (ix / inResolution);
            int yNorm = (iy / inResolution);
            if (ix < inResolution && iy < inResolution)
            {
                auto& c = tileData[((w) * ix / inResolution) + w * ((h) * iy / inResolution)];
                inoutVertices[ix * (inResolution + 1) + iy].Z = interface->Execute_CalcHeight(
                    object, c);
            }
            else if (ix < inResolution)
            {
                inoutVertices[ix * (inResolution + 1) + iy].Z = interface->Execute_CalcHeight(
                    object, bottomData[((w) * ix / inResolution) + 0]);
            }
            else if (iy < inResolution)
            {
                inoutVertices[ix * (inResolution + 1) + iy].Z = interface->Execute_CalcHeight(
                    object, rightData[0 + w * ((h) * iy / inResolution)]);
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
            FVector r;
            FVector b;
            if (ix < inResolution - 1)
            {
                r = inoutVertices[(ix + 1) * (inResolution + 1) + iy] - inoutVertices[ix * (inResolution + 1) + iy];                
            }
            else if (iy < inResolution - 1)
            {
                float hr = Cast<IHeightCalculator>(HeightCalculator.GetObject())->Execute_CalcHeight(
                    HeightCalculator.GetObject(),
                    rightData[((w) * (ix - inResolution + 1) / inResolution) + w * ((h) * iy / inResolution)]);
                r = FVector(delta.X, 0, hr - inoutVertices[ix * (inResolution + 1) + iy].Z);
            }
            else
            {
                float hr = Cast<IHeightCalculator>(HeightCalculator.GetObject())->Execute_CalcHeight(
                    HeightCalculator.GetObject(),
                    brData[((w) * (ix - inResolution + 1) / inResolution) + w * ((h)* (iy - inResolution + 1) / inResolution)]);
                r = FVector(delta.X, 0, hr - inoutVertices[ix * (inResolution + 1) + iy].Z);
            }
            
            if (iy < inResolution - 1)
            {
                b = inoutVertices[ix * (inResolution + 1) + iy + 1] - inoutVertices[ix * (inResolution + 1) + iy];
            }
            else if (ix < inResolution - 1)
            {
                float hb = Cast<IHeightCalculator>(HeightCalculator.GetObject())->Execute_CalcHeight(
                    HeightCalculator.GetObject(),
                    bottomData[((w) * ix / inResolution) + w * ((h) * (iy - inResolution + 1) / inResolution)]);
                b = FVector(0, delta.Y, hb - inoutVertices[ix * (inResolution + 1) + iy].Z);
            }
            else
            {
                float hb = Cast<IHeightCalculator>(HeightCalculator.GetObject())->Execute_CalcHeight(
                    HeightCalculator.GetObject(),
                    brData[((w)* (ix - inResolution + 1) / inResolution) + w * ((h) * (iy - inResolution + 1) / inResolution)]);
                b = FVector(0, delta.Y, hb - inoutVertices[ix * (inResolution + 1) + iy].Z);
            }
            
            outNormals[ix * (inResolution + 1) + iy] = FVector::CrossProduct(r, b).GetSafeNormal();
        }
    }

    TArray<FLinearColor> normalMap;
    float textureSize = (int)inTile->Textures[inChannel]->GetSurfaceHeight();
    normalMap.AddZeroed(textureSize * textureSize);
    delta = (inoutVertices[2 + inResolution] - inoutVertices[0]) / textureSize * inResolution;
    for (int ix = 0; ix < (int)inTile->Textures[inChannel]->GetSurfaceWidth(); ix++)
    {
        for (int iy = 0; iy < (int)inTile->Textures[inChannel]->GetSurfaceHeight(); iy++)
        {
            FVector r;
            FVector b;

            h = Cast<IHeightCalculator>(HeightCalculator.GetObject())->Execute_CalcHeight(HeightCalculator.GetObject(), tileData[iy + ix * textureSize]);
            float hr, hb;
            if (ix < (int)inTile->Textures[inChannel]->GetSurfaceWidth() - 1)
            {
                 hb = Cast<IHeightCalculator>(HeightCalculator.GetObject())->Execute_CalcHeight(HeightCalculator.GetObject(),
                    tileData[iy + (ix + 1)* textureSize]);
            }
            else
            {
                hb = Cast<IHeightCalculator>(HeightCalculator.GetObject())->Execute_CalcHeight(HeightCalculator.GetObject(),
                    bottomData[iy + (0) * textureSize]);
            }

            if (iy < (int)inTile->Textures[inChannel]->GetSurfaceHeight() - 1)
            {
                hr = Cast<IHeightCalculator>(HeightCalculator.GetObject())->Execute_CalcHeight(HeightCalculator.GetObject(),
                    tileData[iy + 1 + ix * textureSize]);

            }
            else
            {
                hr = Cast<IHeightCalculator>(HeightCalculator.GetObject())->Execute_CalcHeight(HeightCalculator.GetObject(),
                    rightData[0 + ix * textureSize]);
            }
            r = FVector(delta.X, 0, hr - h);
            b = FVector(0, delta.Y, hb - h);
            normalMap[ix * textureSize + iy] = FLinearColor(FVector::CrossProduct(r, b).GetSafeNormal() * 0.5f + 0.5f);
            //normalMap[ix * textureSize + iy] = FColor::Red;
        }
    }

    auto normalMapTex = UImageDownloadOverride::LoadImageFromRaw(normalMap, textureSize);
	
    inTile->Textures.Add("Normal", normalMapTex);
	inTile->Material->SetTextureParameterValue(FName(TEXT("Normal")), normalMapTex);
}

// Fill out your copyright notice in the Description page of Project Settings.


#include "TileVisualizer.h"
#include "TilesContainer.h"
#include "Engine/Engine.h"

// Sets default values for this component's properties
UTilesController::UTilesController(const FObjectInitializer& ObjectInitializer) : URuntimeMeshComponent(ObjectInitializer)
{	
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UTilesController::BeginPlay()
{
	Super::BeginPlay();
	ClearMesh();
	CreateMesh();	
}


// Called every frame
void UTilesController::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction * ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);	

	int x0, y0;
	GetMercatorXYFromOffset(FVector::ZeroVector, BaseLevel, x0, y0);
	
	CreateMeshAroundPoint(BaseLevel, x0, y0);

	TArray<FTileCoordinates> pendingDelete;
	TArray<FTileCoordinates, FHeapAllocator> pendingUpdate;	
	for (auto tile : TileIndecies)
	{
		if (SplitTiles.Contains(tile.Key)) continue;
		int x = tile.Key.X;
		int y = tile.Key.Y;
		x /= (1 << (tile.Key.Z - BaseLevel));
		y /= (1 << (tile.Key.Z - BaseLevel));
		
		if (tile.Key.Z == BaseLevel && FMath::Abs(x - x0) > BaseLevelSize || FMath::Abs(y - y0) > BaseLevelSize)
		{
			if (TileLoader->IsTextureLoaded(tile.Key))
			{
				
				pendingDelete.Add(tile.Key);
			}
		} else
		{
			pendingUpdate.AddUnique(tile.Key);
		}
	}
	for (auto meta : pendingDelete)
	{
		if (!TileIndecies.Contains(meta)) continue;
		ClearTileMesh(meta);		
	}	
	for (auto meta : pendingUpdate)
	{
		if (!TileIndecies.Contains(meta)) continue;

		float tilePixelsSize = GetPixelSize(meta);
		if (tilePixelsSize > 256.0f && meta.Z <= MaxLevel)
		{			
			SplitTile(meta);			

		}
		FTileCoordinates parentMeta = FTileCoordinates{ meta.X / 2, meta.Y / 2, meta.Z - 1 };
		float parentTilePixelsSize = GetPixelSize(parentMeta);
		if (parentTilePixelsSize < 200.0f && meta.Z > BaseLevel)
		{

			auto x = meta.X - (meta.X % 2);
			auto y = meta.Y - (meta.Y % 2);

			bool flag = false;
			for (int i = 0; i < 4; i++)
			{
				auto meta1 = FTileCoordinates{ x + i % 2, y + i / 2, meta.Z };
				if (!TileIndecies.Contains(meta1)) 
				{
					flag = true;
				}
			}
			if (!flag)
			{
				CreateTileMesh(parentMeta);
				SplitTiles.Remove(parentMeta);
				for (int i = 0; i < 4; i++)
				{
					auto meta1 = FTileCoordinates{ x + i % 2, y + i / 2, meta.Z };
					ClearTileMesh(meta1);
				}
			}
		}
	}
}

float UTilesController::GetPixelSize(FTileCoordinates meta)
{
	auto pos = GetXYOffsetFromMercatorOffset(meta.Z, meta.X, meta.Y) + GetOwner()->GetActorLocation();
	auto controller =  GEngine->GetFirstLocalPlayerController(GetWorld());
	if (controller)
	{
		auto manager = controller->PlayerCameraManager;
		float dist = (pos - manager->GetCameraLocation()).Size();
		float tan = FMath::Tan(manager->GetCameraCachePOV().DesiredFOV * PI / 180 / 2);
		float viewSize = 2 * dist * tan;
		float tileSize = 360 * EarthOneDegreeLengthOnEquator * FMath::Cos(CenterLat * PI / 180) / (1 << meta.Z);
		float tilePixelsSize = tileSize / viewSize * GEngine->GameViewport->Viewport->GetSizeXY().X;
		return tilePixelsSize;
	}
	return 220; //so we just wont update anything for this tile if cannot find player controller.
}


void UTilesController::CreateMesh()
{
	if (!TileLoader)
	{
		UE_LOG(LogTemp, Error, TEXT("Tile Loader is not initialized"));
	}

	int z = BaseLevel;

	int x0 = GetMercatorXFromDegrees(CenterLon) * (1 << z);
	int y0 = GetMercatorYFromDegrees(CenterLat) * (1 << z);
	
	for (int x = 0; x < BaseLevelSize; x++)
	{
		for (int y = 0; y < BaseLevelSize; y++)
		{
			CreateTileMesh(x + x0 - BaseLevelSize/2, y + y0 - BaseLevelSize/2, z);
		}
	}
}

void UTilesController::ClearMesh()
{
	ClearAllMeshSections();
	TileIndecies.Empty();
}


void UTilesController::CreateMeshAroundPoint(int z, int x0, int y0)
{
	if (!TileLoader)	TileLoader = NewObject<UTileTextureContainer>(this);

	//mesh->ClearAllMeshSections();
	for (int x = 0; x < BaseLevelSize; x++)
	{
		for (int y = 0; y < BaseLevelSize; y++)
		{
			if (!SplitTiles.Contains(FTileCoordinates{ x + x0 - BaseLevelSize / 2, y + y0 - BaseLevelSize / 2, z }))
			{
				CreateTileMesh(x + x0 - BaseLevelSize / 2, y + y0 - BaseLevelSize / 2, z);
			}
		}
	}
}

double UTilesController::EarthRadius  = 6378.137 * 100;
double UTilesController::EarthOneDegreeLengthOnEquator = 111152.8928 * 100;

void UTilesController::GetMercatorXYFromOffset(FVector offsetValue, int z, int& x, int& y)
{
	int dx, dy;
	GetMercatorXYOffsetFromOffset(offsetValue, z, dx, dy);
	int x0 = GetMercatorXFromDegrees(CenterLon) * (1 << z);
	int y0 = GetMercatorYFromDegrees(CenterLat) * (1 << z);

	x = x0 + dx;
	y = y0 + dy;
}

void UTilesController::GetMercatorXYOffsetFromOffset(FVector offsetValue, int z, int& dx, int& dy)
{
	float fdx = offsetValue.X * (1 << z) / 360 / EarthOneDegreeLengthOnEquator / FMath::Cos(CenterLat * PI / 180);
	float fdy = offsetValue.Y * (1 << z) / 360 / EarthOneDegreeLengthOnEquator / FMath::Cos(CenterLat * PI / 180);
	dx = (int)FMath::RoundFromZero(fdx);
	dy = (int)FMath::RoundFromZero(fdy);
}

FVector UTilesController::GetXYOffsetFromMercatorOffset(int z, int x, int y)
{
	int x0 = GetMercatorXFromDegrees(CenterLon) * (1 << z);
	int y0 = GetMercatorYFromDegrees(CenterLat) * (1 << z);
	float fdx = (x - x0) * 360 * EarthOneDegreeLengthOnEquator / (1 << z) * FMath::Cos(CenterLat * PI / 180);
	float fdy = (y - y0) * 360 * EarthOneDegreeLengthOnEquator / (1 << z) * FMath::Cos(CenterLat * PI / 180);
	return FVector(fdx, fdy, 0) + GetOwner()->GetActorLocation();
}

int UTilesController::CreateTileMesh(int x, int y, int z)
{
	auto meta = FTileCoordinates{ x, y, z };
	return CreateTileMesh(meta);
}

int UTilesController::CreateTileMesh(FTileCoordinates meta)
{	
	if (TileIndecies.Contains(meta))
	{
		//mesh->SetMeshSectionVisible(TileIndecies[meta], true);
		return TileIndecies[meta];
	}
	int x = meta.X;
	int y = meta.Y;
	int z = meta.Z;
	TArray<FVector> vertices;
	TArray<FVector> normals;
	TArray<int> triangles;
	TArray<FVector2D> uvs;
	
	float x0 = GetMercatorXFromDegrees(CenterLon) * (1 << z);
	float y0 = GetMercatorYFromDegrees(CenterLat) * (1 << z);
	float size = EarthOneDegreeLengthOnEquator / (1 << z) * 360 * FMath::Cos(CenterLat * PI / 180);
	
	FVector delta((x - x0) * size, (y - y0) * size, 0);
	if (TileMeshResolution < 1) TileMeshResolution = 1;
	for (int ix = 0; ix < TileMeshResolution + 1; ix++)
	{
		for (int iy = 0; iy < TileMeshResolution + 1; iy++)
		{
			vertices.Add(delta + FVector(size * ix / TileMeshResolution, size * iy / TileMeshResolution, 0));
			normals.Add(FVector::UpVector);
			uvs.Add(FVector2D(1.0f * ix / TileMeshResolution, 1.0f * iy / TileMeshResolution));

			if (ix != TileMeshResolution && iy != TileMeshResolution)
			{
				int w = TileMeshResolution + 1;
				triangles.Add(iy * w + ix);				
				triangles.Add(iy * w + ix + 1);
				triangles.Add((iy + 1) * w + ix);

				triangles.Add(iy * w + ix + 1);
				triangles.Add((iy + 1) * w + ix + 1);
				triangles.Add((iy + 1) * w + ix);
			}
		}		
	}


	int sectionIndex = -1;
	if (freeIndices.Num() > 0)
	{
		sectionIndex = freeIndices.Pop();
	}
	else
	{		
		sectionIndex = GetNumSections();
	}	
	CreateMeshSection(sectionIndex, vertices, triangles, normals, uvs, TArray<FColor>(),
	                       TArray<FRuntimeMeshTangent>(), false);
	
	auto tile = TileLoader->GetTileMaterial(x, y, z, TileMaterial, this->GetOwner());
	SetMaterial(sectionIndex, tile->Material);
	
	TileLoader->SetTileActive(meta, true);	
	TileIndecies.Add(meta, sectionIndex);
	Tiles.Add(meta, tile);
	return  sectionIndex;
}

void UTilesController::ClearTileMesh(FTileCoordinates meta)
{
	int index = TileIndecies[meta];
	SetMeshSectionVisible(index, false);
	
	TileLoader->SetTileActive(meta, false);
	freeIndices.Add(index);
	TileIndecies.Remove(meta);
	Tiles[meta]->IsActive = false;
	Tiles[meta]->lastAcessTime = FDateTime::Now();
}

bool UTilesController::IsTileSplit(int x, int y, int z)
{
	return
		TileIndecies.Contains(FTileCoordinates{x * 2, y * 2, z + 1}) &&
		TileIndecies.Contains(FTileCoordinates{x * 2 + 1, y * 2, z + 1}) &&
		TileIndecies.Contains(FTileCoordinates{x * 2, y * 2 + 1, z + 1}) &&
		TileIndecies.Contains(FTileCoordinates{x * 2 + 1, y * 2 + 1, z + 1});
}

void UTilesController::SplitTile(FTileCoordinates m)
{
	SplitTile(m.X, m.Y, m.Z);
}

void UTilesController::SplitTile(int x, int y, int z)
{
	auto parentMeta = FTileCoordinates{ x, y, z };
	auto childMeta1 = FTileCoordinates{ x * 2, y * 2, z + 1 };
	auto childMeta2 = FTileCoordinates{ x * 2 + 1, y * 2, z + 1 };
	auto childMeta3 = FTileCoordinates{ x * 2, y * 2 + 1, z + 1 };
	auto childMeta4 = FTileCoordinates{ x * 2 + 1, y * 2 + 1, z + 1 };

	TileLoader->GetTileMaterial(childMeta1, TileMaterial, GetOwner());
	TileLoader->GetTileMaterial(childMeta2, TileMaterial, GetOwner());
	TileLoader->GetTileMaterial(childMeta3, TileMaterial, GetOwner());
	TileLoader->GetTileMaterial(childMeta4, TileMaterial, GetOwner());

	if (TileLoader->IsTextureLoaded(childMeta1) && TileLoader->IsTextureLoaded(childMeta2) &&
		TileLoader->IsTextureLoaded(childMeta3) && TileLoader->IsTextureLoaded(childMeta4))
	{
		
		
		CreateTileMesh(childMeta1);
		CreateTileMesh(childMeta2);
		CreateTileMesh(childMeta3);
		CreateTileMesh(childMeta4);
		ClearTileMesh(parentMeta);
		SplitTiles.Add(parentMeta);
		TileIndecies.Remove(parentMeta);
		Tiles[parentMeta]->IsActive = false;
		Tiles[parentMeta]->lastAcessTime = FDateTime::Now();
	}
	
	
	
}


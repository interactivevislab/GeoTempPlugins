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
}


// Called every frame
void UTilesController::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction * ThisTickFunction)
{
	
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);	
	return;
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
				BeginCreateTileMesh(parentMeta);
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

float UTilesController::GetPixelSize(FTileCoordinates inTileCoords)
{
	auto pos = GetXYOffsetFromMercatorOffset(inTileCoords.Z, inTileCoords.X, inTileCoords.Y) + GetOwner()->GetActorLocation();
	auto controller =  GEngine->GetFirstLocalPlayerController(GetWorld());
	if (controller)
	{
		auto manager = controller->PlayerCameraManager;
		float dist = (pos - manager->GetCameraLocation()).Size();
		float tan = FMath::Tan(manager->GetCameraCachePOV().DesiredFOV * PI / 180 / 2);
		float viewSize = 2 * dist * tan;
		float tileSize = 360 * EarthOneDegreeLengthOnEquator * FMath::Cos(CenterLat * PI / 180) / (1 << inTileCoords.Z);
		float tilePixelsSize = tileSize / viewSize * GEngine->GameViewport->Viewport->GetSizeXY().X;
		return tilePixelsSize;
	}
	return 220; //so we just wont update anything for this tile if cannot find player controller.
}


void UTilesController::EditorTick_Implementation(float inDeltaTime)
{
	if (NeedInitOnTick && AreTilesLoaded)
	{
		ClearMesh();
		CreateMesh();
		NeedInitOnTick = false;
	}
	if (TileLoader)
	{
		Cast<IEditorTickable>(TileLoader)->Execute_EditorTick(TileLoader, inDeltaTime);
	}
}

void UTilesController::PostLoad()
{
	Super::PostLoad();
	NeedInitOnTick = true;
}


void UTilesController::CreateMesh()
{
	if (!TileLoader)
	{
		UE_LOG(LogTemp, Error, TEXT("Tile Loader is not initialized"));
	}

	int z = BaseLevel;
	TileLoader->ElevationChannel = ElevationChannel;
	int x0 = GetMercatorXFromDegrees(CenterLon) * (1 << z);
	int y0 = GetMercatorYFromDegrees(CenterLat) * (1 << z);
	
	for (int x = 0; x < BaseLevelSize; x++)
	{
		for (int y = 0; y < BaseLevelSize; y++)
		{
			BeginCreateTileMesh(x + x0 - BaseLevelSize/2, y + y0 - BaseLevelSize/2, z);
		}
	}

	AreTilesLoaded = true;
}

void UTilesController::ClearMesh()
{
	ClearAllMeshSections();
	freeIndices.Empty();
	SplitTiles.Empty();
	ReservedIndecies.Empty();
	TileIndecies.Empty();

	AreTilesLoaded = false;
}


void UTilesController::CreateMeshAroundPoint(int inZoom, int inX, int inY)
{
	if (!TileLoader)	TileLoader = NewObject<UTileTextureContainer>(this);
	TileLoader->ElevationChannel = ElevationChannel;
	//mesh->ClearAllMeshSections();
	for (int x = 0; x < BaseLevelSize; x++)
	{
		for (int y = 0; y < BaseLevelSize; y++)
		{
			if (!SplitTiles.Contains(FTileCoordinates{ inX + x - BaseLevelSize / 2, inX + y - BaseLevelSize / 2, inZoom }))
			{
				BeginCreateTileMesh(inX + x - BaseLevelSize / 2, inY + y - BaseLevelSize / 2, inZoom);
			}
		}
	}
	AreTilesLoaded = true;
}

double UTilesController::EarthRadius  = 6378.137 * 100;
double UTilesController::EarthOneDegreeLengthOnEquator = 111152.8928 * 100;

void UTilesController::GetMercatorXYFromOffset(FVector inOffsetVector, int inZ, int& outX, int& outY)
{
	int dx, dy;
	GetMercatorXYOffsetFromOffset(inOffsetVector, inZ, dx, dy);
	int x0 = GetMercatorXFromDegrees(CenterLon) * (1 << inZ);
	int y0 = GetMercatorYFromDegrees(CenterLat) * (1 << inZ);

	outX = x0 + dx;
	outY = y0 + dy;
}

void UTilesController::GetMercatorXYOffsetFromOffset(FVector inOffsetVector, int inZ, int& dx, int& dy)
{
	float fdx = inOffsetVector.X * (1 << inZ) / 360 / EarthOneDegreeLengthOnEquator / FMath::Cos(CenterLat * PI / 180);
	float fdy = inOffsetVector.Y * (1 << inZ) / 360 / EarthOneDegreeLengthOnEquator / FMath::Cos(CenterLat * PI / 180);
	dx = (int)FMath::RoundFromZero(fdx);
	dy = (int)FMath::RoundFromZero(fdy);
}

FVector UTilesController::GetXYOffsetFromMercatorOffset(int inZ, int inX, int inY)
{
	int x0 = GetMercatorXFromDegrees(CenterLon) * (1 << inZ);
	int y0 = GetMercatorYFromDegrees(CenterLat) * (1 << inZ);
	float fdx = (inX - x0) * 360 * EarthOneDegreeLengthOnEquator / (1 << inZ) * FMath::Cos(CenterLat * PI / 180);
	float fdy = (inY - y0) * 360 * EarthOneDegreeLengthOnEquator / (1 << inZ) * FMath::Cos(CenterLat * PI / 180);
	return FVector(fdx, fdy, 0) + GetOwner()->GetActorLocation();
}

void UTilesController::CreateTileMesh(UTileData* inTile)
{
	auto& meta = inTile->Meta;	
	if (TileIndecies.Contains(meta))
	{
		return;// TileIndecies[meta];
	}
	int sectionIndex = ReservedIndecies[meta];
	TileIndecies.Add(meta, sectionIndex);
	ReservedIndecies.Remove(meta);
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

	if (ElevationChannel.IsEmpty())
	{
		FVector delta((x - x0) * size, (y - y0) * size, 0);
		if (TileMeshResolution < 1) TileMeshResolution = 1;
		for (int ix = 0; ix < 2; ix++)
		{
			for (int iy = 0; iy < 2; iy++)
			{
				vertices.Add(delta + FVector(size * ix, size * iy, 0));
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
	}
	else
	{
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
		GeometryGenerator->GenerateVertices(inTile, ElevationChannel, TileMeshResolution, vertices, normals);
	}
	
	CreateMeshSection(sectionIndex, vertices, triangles, normals, uvs, TArray<FColor>(), TArray<FRuntimeMeshTangent>(), false);
	SetMaterial(sectionIndex, inTile->Material);
}


int UTilesController::BeginCreateTileMesh(int inX, int inY, int inZ)
{
	auto meta = FTileCoordinates{ inX, inY, inZ };
	return BeginCreateTileMesh(meta, true);
}

int UTilesController::BeginCreateTileMesh(FTileCoordinates inTileCoords, bool inInitNeighbours)
{	
	if (TileIndecies.Contains(inTileCoords))
	{
		return TileIndecies[inTileCoords];
	}
	else if (ReservedIndecies.Contains(inTileCoords))
	{
		return ReservedIndecies.FindRef(inTileCoords);
	}
	int sectionIndex = -1;
	if (freeIndices.Num() > 0)
	{
		sectionIndex = freeIndices.Pop();
	}
	else
	{		
		sectionIndex = GetNumSections() + ReservedIndecies.Num();
	}
	UTileData* tile;

	ReservedIndecies.Add(inTileCoords, sectionIndex);
	tile = TileLoader->GetTileMaterial(inTileCoords, TileMaterial, this->GetOwner());
	
	
	tile->OnTileLoadWithNeighbors.Clear();
	tile->OnTileLoadWithNeighbors.AddDynamic(this, &UTilesController::CreateTileMesh);

	if (inInitNeighbours)
	{
		BeginCreateTileMesh(FTileCoordinates{ inTileCoords.X + 1, inTileCoords.Y    , inTileCoords.Z }, false);
		BeginCreateTileMesh(FTileCoordinates{ inTileCoords.X    , inTileCoords.Y + 1, inTileCoords.Z }, false);
		BeginCreateTileMesh(FTileCoordinates{ inTileCoords.X + 1, inTileCoords.Y + 1, inTileCoords.Z }, false);
	}
	
	tile->CheckLoaded();
	TileLoader->SetTileActive(inTileCoords, true);		
	Tiles.Add(inTileCoords, tile);	
	return sectionIndex;
}

void UTilesController::ClearTileMesh(FTileCoordinates inTileCoords)
{
	int index = TileIndecies[inTileCoords];
	SetMeshSectionVisible(index, false);
	
	TileLoader->SetTileActive(inTileCoords, false);
	freeIndices.Add(index);
	TileIndecies.Remove(inTileCoords);
	Tiles[inTileCoords]->IsActive = false;
	Tiles[inTileCoords]->lastAcessTime = FDateTime::Now();
}

bool UTilesController::IsTileSplit(int inX, int inY, int inZ)
{
	return
		TileIndecies.Contains(FTileCoordinates{inX * 2, inY * 2, inZ + 1}) &&
		TileIndecies.Contains(FTileCoordinates{inX * 2 + 1, inY * 2, inZ + 1}) &&
		TileIndecies.Contains(FTileCoordinates{inX * 2, inY * 2 + 1, inZ + 1}) &&
		TileIndecies.Contains(FTileCoordinates{inX * 2 + 1, inY * 2 + 1, inZ + 1});
}

void UTilesController::SplitTile(FTileCoordinates m)
{
	SplitTile(m.X, m.Y, m.Z);
}

void UTilesController::SplitTile(int inX, int inY, int inZ)
{
	auto parentMeta = FTileCoordinates{ inX, inY, inZ };
	auto childMeta1 = FTileCoordinates{ inX * 2, inY * 2, inZ + 1 };
	auto childMeta2 = FTileCoordinates{ inX * 2 + 1, inY * 2, inZ + 1 };
	auto childMeta3 = FTileCoordinates{ inX * 2, inY * 2 + 1, inZ + 1 };
	auto childMeta4 = FTileCoordinates{ inX * 2 + 1, inY * 2 + 1, inZ + 1 };

	TileLoader->GetTileMaterial(childMeta1, TileMaterial, GetOwner());
	TileLoader->GetTileMaterial(childMeta2, TileMaterial, GetOwner());
	TileLoader->GetTileMaterial(childMeta3, TileMaterial, GetOwner());
	TileLoader->GetTileMaterial(childMeta4, TileMaterial, GetOwner());

	if (TileLoader->IsTextureLoaded(childMeta1) && TileLoader->IsTextureLoaded(childMeta2) &&
		TileLoader->IsTextureLoaded(childMeta3) && TileLoader->IsTextureLoaded(childMeta4))
	{
		
		
		BeginCreateTileMesh(childMeta1);
		BeginCreateTileMesh(childMeta2);
		BeginCreateTileMesh(childMeta3);
		BeginCreateTileMesh(childMeta4);
		ClearTileMesh(parentMeta);
		SplitTiles.Add(parentMeta);
		TileIndecies.Remove(parentMeta);
		Tiles[parentMeta]->IsActive = false;
		Tiles[parentMeta]->lastAcessTime = FDateTime::Now();
	}
	
	
	
}


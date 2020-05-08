#include "TilesBasics.h"


#include "TilesContainer.h"



void UTileData::CheckLoaded()
{
	bool allLoaded = true;

	auto left		= GetLeftNeighbor();
	auto top		= GetTopNeighbor();
	auto topLeft	= GetTopLeftNeighbor();	
	for (auto& kv : IsLoaded)
	{
		allLoaded &= kv.Value;
		if (kv.Value) OnTextureLoad.Broadcast(this, kv.Key);
		if (left) 
		{
			left->		IsRightLoaded.Add(kv.Key, kv.Value);
		}
		if (top) 
		{
			top->		IsBottomLoaded.Add(kv.Key, kv.Value);
		}
		if (topLeft)
		{
			topLeft->	IsBottomRightLoaded.Add(kv.Key, kv.Value);
		}
	}
	
	if (left) left->		CheckNeighborLoaded();
	if (top) top->		CheckNeighborLoaded();
	if (topLeft) topLeft->	CheckNeighborLoaded();
	CheckNeighborLoaded();
	if (allLoaded)
	{
		OnTileLoad.Broadcast(this);
	}
	
	
}

void UTileData::CheckNeighborLoaded()
{	
	bool allLoaded = true;
	if (IsLoaded.Num() == 0)
	{
		return;
	}
	for (auto& kv : IsLoaded)
	{
		auto loaded = kv.Value && IsRightLoaded.FindRef(kv.Key) && IsBottomLoaded.FindRef(kv.Key) && IsBottomRightLoaded
			.FindRef(kv.Key);
		allLoaded &= (loaded);
		if (loaded)
		{
			OnTextureLoadWithNeighbours.Broadcast(this, kv.Key);
		}
	}
	if (allLoaded)
	{
		OnTileLoadWithNeighbors.Broadcast(this);
	}
}

UTileData* UTileData::GetRightNeighbor()
{
	return Container->CachedTiles.FindRef(FTileCoordinates{Meta.X + 1, Meta.Y    , Meta.Z});
}

UTileData* UTileData::GetLeftNeighbor()
{
	return Container->CachedTiles.FindRef(FTileCoordinates{Meta.X - 1, Meta.Y    , Meta.Z});
}

UTileData* UTileData::GetTopNeighbor()
{
	return Container->CachedTiles.FindRef(FTileCoordinates{Meta.X    , Meta.Y - 1, Meta.Z});
}

UTileData* UTileData::GetBottomNeighbor()
{
	return Container->CachedTiles.FindRef(FTileCoordinates{Meta.X    , Meta.Y + 1, Meta.Z});
}

UTileData* UTileData::GetTopLeftNeighbor()
{
	return Container->CachedTiles.FindRef(FTileCoordinates{Meta.X - 1, Meta.Y - 1, Meta.Z});
}

UTileData* UTileData::GetBottomRightNeighbor()
{
	return Container->CachedTiles.FindRef(FTileCoordinates{Meta.X + 1, Meta.Y + 1, Meta.Z});
}

UTileData* UTileData::GetBottomLeftNeighbor()
{
	return Container->CachedTiles.FindRef(FTileCoordinates{Meta.X - 1, Meta.Y + 1, Meta.Z});
}

UTileData* UTileData::GetTopRightNeighbor()
{
	return Container->CachedTiles.FindRef(FTileCoordinates{Meta.X + 1, Meta.Y - 1, Meta.Z});
}



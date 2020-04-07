#pragma once

#include "CoreMinimal.h"

#include "RoadsData.h"

#include "RoadLoader.generated.h"


UCLASS(BlueprintType, Meta = (BlueprintSpawnableComponent))
class GEOTEMPCORE_API URoadLoader : public UObject
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, BlueprintPure)
	static FRoadNetwork ProcessPostGisRoadNetwork(FPostGisRoadNetwork inRoadNetwork);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	static FRoadNetwork ProcessOsmRoadNetwork(FOsmRoadNetwork inRoadNetwork);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	static FRoadNetwork GetRoadNetworkForYear(FRoadNetwork inFullRoadNetwork, int inYear);

private:

	template<typename RoadNetworkType>
	static TArray<FRoadSegment> GetRoadSegments(RoadNetworkType inRoadNetwork)
	{
		return TArray<FRoadSegment>();
	}


	template<>
	static TArray<FRoadSegment> GetRoadSegments<FPostGisRoadNetwork>(FPostGisRoadNetwork inRoadNetwork)
	{
		TArray<FRoadSegment> segments;
		for (auto postGisSegmentPair : inRoadNetwork.Segments)
		{
			auto postGisSegment = postGisSegmentPair.Value;

			FRoadSegment segment;
			segment.Type = postGisSegment.Highway;
			segment.Width = postGisSegment.Lanes * postGisSegment.LaneWidth;
			segment.Lanes = postGisSegment.Lanes;
			segment.StartYear = postGisSegment.YearStart;
			segment.EndYear = postGisSegment.YearEnd;
			segment.Change = postGisSegment.Change;
			segment.AllPoints = postGisSegment.Line.AllPoints;

			segments.Add(segment);
		}

		return segments;
	}


	template<>
	static TArray<FRoadSegment> GetRoadSegments<FOsmRoadNetwork>(FOsmRoadNetwork inRoadNetwork)
	{
		TArray<FRoadSegment> segments;
		for (auto osmSegmentPair : inRoadNetwork.Segments)
		{
			auto osmSegment = osmSegmentPair.Value;

			//TODO: add tag processing for non-constannt values
			FRoadSegment segment;
			segment.Type = EHighwayType::Auto;
			segment.Width = 7;
			segment.Lanes = 2;
			segment.StartYear = 0;
			segment.EndYear = 3000;
			segment.Change = "";
			segment.AllPoints = osmSegment.Points;

			segments.Add(segment);
		}

		return segments;
	}


	template<typename RoadNetworkType>
	static FRoadNetwork ProcessRoadNetwork(RoadNetworkType inRoadNetwork)
	{
		TMap<int, FRoadSegment> segments;
		TMap<int, FCrossroad>	crossroads;
		TMap<FVector, int>		crossroadIds;

		auto inputSegments = GetRoadSegments(inRoadNetwork);

		int nextSegmentId = 0;
		int nextCrossroadId = 0;

		for (auto segment : inputSegments)
		{
			auto pointStart = segment.AllPoints[0];
			auto pointEnd = segment.AllPoints[segment.AllPoints.Num() - 1];

			FCrossroad* crossroadStart;
			FCrossroad* crossroadEnd;

			auto ptr = crossroadIds.Find(pointStart);
			if (ptr == nullptr)
			{
				crossroadIds.Add(pointStart, nextCrossroadId);
				crossroadStart = &(crossroads.Add(nextCrossroadId, FCrossroad{ pointStart }));
				segment.StartCrossroadId = nextCrossroadId++;
			}
			else
			{
				segment.StartCrossroadId = *ptr;
				crossroadStart = crossroads.Find(*ptr);
			}

			ptr = crossroadIds.Find(pointEnd);
			if (ptr == nullptr)
			{
				crossroadIds.Add(pointEnd, nextCrossroadId);
				crossroadEnd = &(crossroads.Add(nextCrossroadId, FCrossroad{ pointEnd }));
				segment.EndCrossroadId = nextCrossroadId++;
			}
			else
			{
				segment.EndCrossroadId = *ptr;
				crossroadEnd = crossroads.Find(*ptr);
			}

			crossroadStart->Roads.Add(nextSegmentId, segment.EndCrossroadId);
			crossroadEnd->Roads.Add(nextSegmentId, segment.StartCrossroadId);

			segments.Add(nextSegmentId++, segment);
		}

		return FRoadNetwork{ segments, crossroads };
	}
};

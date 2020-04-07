#pragma once

#include "CoreMinimal.h"

#include <vector>

#include "Contour.Generated.h"


USTRUCT(BlueprintType)
struct GEOTEMPCORE_API FContour
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
	TArray<FVector> Points;

	FContour();
	FContour(std::vector<FVector> initPoints);
	FContour(TArray<FVector> initPoints);

	int LeftmostIndex()		const;
	int RightmostIndex()	const;
	int TopmostIndex()		const;
	int BottommostIndex()	const;

	//returns true if contour was reverted
	bool FixClockwise(bool reverse = false);

	void Cleanup();

	inline FContour RemoveCollinear(float treshold = 0.001f) const;
	static FContour RemoveCollinear(FContour contour, float threshold = 0.001f);

	inline FContour MakeConvex() const;
	static FContour MakeConvex(FContour contour);
};

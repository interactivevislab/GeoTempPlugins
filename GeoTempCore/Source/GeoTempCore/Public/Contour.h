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

	//checks and fix contour direction if necessary. Updates contour in place. Returns true if contour was reverted
	bool FixClockwise(bool reverse = false);

	///Removes last point if this contour has repeating point to indicate loop
	void FixLoop();;

	//returns true if contour violates clockwise rule without applying any changes to the contour
	bool IsNotClockwise(bool reverse = false);

	void Cleanup();	

	///Creates copy of this contour and removes points which are collinear to neighbors or near to these
	FContour RemoveCollinear(float treshold = 0.001f) const;
	static FContour RemoveCollinear(FContour contour, float threshold = 0.001f);

	///Creates copy of this contour and removes points to make it convex
	FContour MakeConvex() const;
	static FContour MakeConvex(FContour contour);
};

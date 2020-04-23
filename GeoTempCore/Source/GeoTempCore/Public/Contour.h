#pragma once

#include "CoreMinimal.h"

#include <vector>

#include "Contour.Generated.h"

/** Struct to store contours and lines data */
USTRUCT(BlueprintType)
struct GEOTEMPCORE_API FContour
{
	GENERATED_BODY()

	/** Points of this contour */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
	TArray<FVector> Points;

	FContour();
	/** Initializes this contour from Unreal array */
	FContour(TArray<FVector> initPoints);

	/** returns index of point with minimal X */
	int LeftmostIndex()		const;
	/** returns index of point with maximal X */
	int RightmostIndex()	const;
	/** returns index of point with minimal Y */
	int TopmostIndex()		const;
	/** returns index of point with maximal Y */
	int BottommostIndex()	const;

	/** checks and fix contour direction if necessary. Updates contour in place. Returns true if contour was reverted
	 * @param reverse Maintain clockwise or counter-clockwise order
	 */
	bool FixClockwise(bool reverse = false);

	/**Removes last point if this contour has repeating point to indicate loop*/
	void FixLoop();;

	/**returns true if contour violates clockwise rule without applying any changes to the contour
	 * @param reverse Maintain clockwise or counter-clockwise order
	 */
	bool IsNotClockwise(bool reverse = false);

	/** Removes all duplicate points and performs other cleaning */
	void Cleanup();	

	/** Creates copy of this contour and removes points which are collinear to neighbors or near to these
	 * @param treshold which value of cross product is considered as near enough to zero
	 */
	FContour RemoveCollinear(float treshold = 0.001f) const;

	/** Creates copy of input contour and removes points which are collinear to neighbors or near to these
	 * @param treshold which value of cross product is considered as near enough to zero
	 */
	static FContour RemoveCollinear(FContour contour, float threshold = 0.001f);

	/** Creates copy of this contour and removes points to make it convex */
	FContour MakeConvex() const;
	/** Creates copy of input contour and removes points to make it convex */
	static FContour MakeConvex(FContour contour);
};

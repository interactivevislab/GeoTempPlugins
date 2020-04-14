#pragma once

#include "CoreMinimal.h"

#include <vector>
#include <string>

#include "Contour.h"

#include "Basics.Generated.h"


#ifndef PI
#define PI (3.1415926535897932)
#endif

/** routine to call libigl::tiangle with our data
 * @param inOuter				list of outer contours
 * @param inInner				list of inner contours aka holes
 * @param outPoints				out vertex list of generated triangulation
 * @param outTriangles			out index list of generated triangulation
 * @param inFlags				list of flags to configure triangulator
 * @param inOtherLines			other lines that should be considered by triangulation algorithm
 * @param outContourPointsNum	index of first point of triangulation which not contained in outer contours
 */
void GEOTEMPCORE_API  Triangulate(const TArray<FContour>& inOuter, const TArray<FContour>& inInner, TArray<FVector>& outPoints,
	TArray<int>& outTriangles, FString inFlags, const TArray<FContour>& inOtherLines, int& outContourPointsNum);


/** routine to call libigl::tiangle with our data
 * @param inOuter				list of outer contours
 * @param inInner				list of inner contours aka holes
 * @param outPoints				out vertex list of generated triangulation
 * @param outTriangles			out index list of generated triangulation
 * @param inFlags				list of flags to configure triangulator
 */
void GEOTEMPCORE_API  Triangulate(const TArray<FContour>& inOuter, const TArray<FContour>& inInner, TArray<FVector>& outPoints,
	TArray<int>& outTriangles, FString inFlags = "");

/** \enum ProjectionType
 * Enum to store different types of cartographic projections supported by our loaders
 */
UENUM(BlueprintType)
enum class ProjectionType : uint8
{
	WGS84_PsevdoMerkator	UMETA(DisplayName = "WGS84 Psevdo Merkator"),	/**< Psevdo Merkator */
	WGS84					UMETA(DisplayName = "WGS84"),					/**< WGS84 */
	LOCAL_METERS			UMETA(DisplayName = "Local meter coordinates"),	/**< Input in scene coordinates*/
};


/** Struct to store geodetic coordinates of origin point */
USTRUCT(BlueprintType)
struct GEOTEMPCORE_API FGeoCoords
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	ProjectionType Projection;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float ZeroLon;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float ZeroLat;

	FGeoCoords();
	FGeoCoords(ProjectionType projection, float zeroLon, float zeroLat);
};


UCLASS()
class GEOTEMPCORE_API UGeoHelpers : public UObject
{
	GENERATED_BODY()

public:

	static const double EARTH_RADIUS;
	static const double SCALE_MULT;

	static double DegreesToRadians(double inAngle);
	static double RadiansToDegrees(double inAngle);

	static FVector GetLocalCoordinates(double inX, double inY, double inZ, FGeoCoords inGeoCoords);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Default")
	static FVector2D ConvertToLonLat(float inX, float inY, FGeoCoords inGeoCoords);
};

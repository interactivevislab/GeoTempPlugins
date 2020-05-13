#pragma once

#include "CoreMinimal.h"

#include <vector>
#include <string>

#include "Contour.h"

#include "Basics.Generated.h"


#ifndef PI
#define PI (3.1415926535897932)
#endif

/** routine to call libigl-triangle with our data
 * @param inOuter                list of outer contours
 * @param inInner                list of inner contours a.k.a. holes
 * @param outPoints                out vertex list of generated triangulation
 * @param outTriangles            out index list of generated triangulation
 * @param inFlags                list of flags to configure triangulator
 * @param inOtherLines            other lines that should be considered by triangulation algorithm
 * @param outContourPointsNum    index of first point of triangulation which not contained in outer contours
 */
void GEOTEMPCORE_API  Triangulate(const TArray<FContour>& inOuter, const TArray<FContour>& inInner, TArray<FVector>& outPoints,
    TArray<int>& outTriangles, FString inFlags, const TArray<FContour>& inOtherLines, int& outContourPointsNum);


/** routine to call libigl-triangle with our data
 * @param inOuter                list of outer contours
 * @param inInner                list of inner contours a.k.a. holes
 * @param outPoints                out vertex list of generated triangulation
 * @param outTriangles            out index list of generated triangulation
 * @param inFlags                list of flags to configure triangulator
 */
void GEOTEMPCORE_API  Triangulate(const TArray<FContour>& inOuter, const TArray<FContour>& inInner, TArray<FVector>& outPoints,
    TArray<int>& outTriangles, FString inFlags = "");

/** \enum ProjectionType
 * Enum to store different types of cartographic projections supported by our loaders
 */
UENUM(BlueprintType)
enum class ProjectionType : uint8
{
    WGS84_PsevdoMerkator    UMETA(DisplayName = "WGS84 Psevdo Merkator"),    /**< Psevdo Merkator */
    WGS84                    UMETA(DisplayName = "WGS84"),                    /**< WGS84 */
    LOCAL_METERS            UMETA(DisplayName = "Local meter coordinates"),    /**< Input in scene coordinates*/
};


/** Struct to match scene with source projection coordinates */
USTRUCT(BlueprintType)
struct GEOTEMPCORE_API FGeoCoords
{
    GENERATED_BODY()

public:

    /** Coordinatesystem the data is stored in source */
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    ProjectionType Projection = ProjectionType::WGS84;

    /** x coordinate of origin point in source projection */
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    float OriginLon;

    /** y coordinate of origin point in source projection */
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    float OriginLat;

    FGeoCoords();
    FGeoCoords(ProjectionType projection, float zeroLon, float zeroLat);
};


UCLASS()
class GEOTEMPCORE_API UGeoHelpers : public UObject
{
    GENERATED_BODY()

public:
    /** Earth radius in meters */
    static const double EARTH_RADIUS;
    
    /** Scale multiplier to convert meters to scene units */
    static const double SCALE_MULT;

    /** Routine to convert degrees to radians */
    static double DegreesToRadians(double inAngle);
    /** Routine to convert radians to degrees */
    static double RadiansToDegrees(double inAngle);

    /** Routine to get scene coordinates from source coordinates
     * @param inX x coordinate in source projection
     * @param inY y coordinate in source projection
     * @param inZ z coordinate in source projection
     * @param inGeoCoords scene origin coordinate in source projection
     * @see FGeoCoords
     * @return FVector of scene coordinates
     */
    static FVector GetLocalCoordinates(double inX, double inY, double inZ, FGeoCoords inGeoCoords);

    /** Routine to convert scene coorinates to source coordinates
     * @param inX x coordinate of point in scene
     * @param inY y coordinate of point in scene
     * @param inGeoCoords scene origin coordinate in source projection
     * @see FGeoCoords
     * @return FVector2D of coordinates in source projection
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Default")
    static FVector2D ConvertToLonLat(float inX, float inY, FGeoCoords inGeoCoords);
};

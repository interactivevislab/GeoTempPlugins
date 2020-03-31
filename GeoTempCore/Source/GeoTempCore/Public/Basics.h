#pragma once

#include "CoreMinimal.h"

#include <vector>
#include <string>

#include "Contour.h"

#include "Basics.Generated.h"


#ifndef PI
#define PI (3.1415926535897932)
#endif


void GEOTEMPCORE_API  Triangulate(TArray<FContour>& outOuter, TArray<FContour>& outInner, std::vector<FVector>& outPoints,
	std::vector<int>& outTriangles, std::string inFlags, TArray<FContour> inOtherLines, int& outContourPointsNum);
void GEOTEMPCORE_API  Triangulate(TArray<FContour>& outOuter, TArray<FContour>& outInner, std::vector<FVector>& outPoints,
	std::vector<int>& outTriangles, std::string inFlags = "");


UENUM(BlueprintType)
enum class ProjectionType : uint8
{
	WGS84_PsevdoMerkator	UMETA(DisplayName = "WGS84 Psevdo Merkator"),
	WGS84					UMETA(DisplayName = "WGS84"),
	LOCAL_METERS			UMETA(DisplayName = "Local meter coordinates"),
};


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

	inline static double DegreesToRadians(double inAngle);
	inline static double RadiansToDegrees(double inAngle);

	static FVector GetLocalCoordinates(double inX, double inY, double inZ, FGeoCoords inGeoCoords);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Default")
	static FVector2D ConvertToLonLat(float inX, float inY, FGeoCoords inGeoCoords);
};
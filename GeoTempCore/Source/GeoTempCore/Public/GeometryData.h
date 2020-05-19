#pragma once

#include "CoreMinimal.h"

#include "Basics.h"

#include "GeometryData.generated.h"

/** Container for binary geometry which can be obtained from different data sources */
USTRUCT(BlueprintType)
struct GEOTEMPCORE_API FWkbEntity
{
	GENERATED_BODY()

	/** Binary array of geometry data */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
	TArray<uint8> Geometry;

	/** Additional tags and properties loaded alongside geometry data */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
	TMap<FString, FString> Tags;
};


/** Container for storing contour array in UE Containers */
USTRUCT(BlueprintType)
struct GEOTEMPCORE_API FLinesData
{
	GENERATED_BODY()

public:
	/** List of contours to store */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
	TArray<FContour> Lines;
};


/** Container for storing georeferenced multipolygon data */
USTRUCT(BlueprintType)
struct GEOTEMPCORE_API FMultipolygonData
{
	GENERATED_BODY()

public:

	/**Scene-projection matching
	 * @see FGeoCoords
	 */	
	FGeoCoords Origin;

	/** List of outer contours of multipolygon */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
	TArray<FContour> Outer;

	/** List of inner contours (a.k.a. holes) of multipolygon */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
	TArray<FContour> Holes;

	/** Additional tags and properties loaded alongside geometry data */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
	TMap<FString, FString> Tags;

	/** Append contours from another multipolygon*/
	void Append(const FMultipolygonData& inOther);

	/** Parse point from binary array, starting with offset. Uses geoCoordinates of this contour
	 * @param inArray binary array of geometry data
	 * @param outOffset current position of cursor in the array
	 * @param inHeight elevation with which initialize the point
	 *
	 * @return coordinates of result point
	 */
	FVector BinaryParsePoint(uint8* inArray, int& outOffset, float inHeight = 0);
	
	/** Parse point from binary array, starting with offset. Static version that requires geoCoordinates input
	 * @param inArray binary array of geometry data
	 * @param outOffset current position of cursor in the array
	 * @param inGeoCoords Scene-projection matching structure
	 * @param inHeight elevation with which initialize the point
	 *
	 * @return coordinates of result point
	 */
	static FVector BinaryParsePoint(uint8* inArray, int& outOffset, FGeoCoords inGeoCoords, float inHeight = 0);

	/** Parse curve from binary array, starting with offset. Uses geoCoordinates of this contour
	 * @param inArray binary array of geometry data
	 * @param outOffset current position of cursor in the array
	 * @param inSkipBOM should the data contain Bite-Order mark under cursor
	 * @param inHeight elevation with which initialize the point
	 *
	 * @return coordinates of result point
	 */
	TArray<FVector> BinaryParseCurve(uint8* inArray, int& outOffset,
		bool inSkipBOM = false, float inHeight = 0);
	
	/** Parse curve from binary array, starting with offset. Static version that requires geoCoordinates input
	 * @param inArray binary array of geometry data
	 * @param outOffset current position of cursor in the array
	 * @param inGeoCoords Scene-projection matching structure
	 * @param inSkipBOM should the data contain Byte-Order mark under cursor
	 * @param inHeight elevation with which initialize the point
	 *
	 * @return result array of points
	 */
	static TArray<FVector> BinaryParseCurve(uint8* inArray, int& outOffset, FGeoCoords inGeoCoords,
		bool inSkipBOM = false, float inHeight = 0);

	/** Parse polygon from binary array, starting with offset. Uses geoCoordinates of this contour
	 * @param inArray binary array of geometry data
	 * @param outOffset current position of cursor in the array
	 * @param inSkipBOM should the data contain Byte-Order mark under cursor
	 * @param inHeight elevation with which initialize the point
	 *
	 * @return result multipolygon
	 */
	FMultipolygonData BinaryParsePolygon(uint8* inArray, int& outOffset,
		bool inSkipBOM, float inHeight = 0);

	/** Parse polygon from binary array, starting with offset. Static version that requires geoCoordinates input
	 * @param inArray binary array of geometry data
	 * @param outOffset current position of cursor in the array
	 * @param inGeoCoords Scene-projection matching structure
	 * @param inSkipBOM should the data contain Byte-Order mark under cursor
	 * @param inHeight elevation with which initialize the point
	 *
	 * @return result multipolygon
	 */
	static FMultipolygonData BinaryParsePolygon(uint8* inArray, int& outOffset, FGeoCoords inGeoCoords,
		bool inSkipBOM, float inHeight = 0);
};

UCLASS()
class GEOTEMPCORE_API UGeometryHelpers : public UObject
{
	GENERATED_BODY()

public:
	/** Get coordinates of lines intersection
	 * @param inFirstLineStart start point of the first line
	 * @param inFirstLineEnd end point of the first line
	 * @param inSecondLineStart start point of the second line
	 * @param inSecondLineEnd end point of the second line
	 * @return FVector point of lines intersection (returns FVector(FLT_MAX, FLT_MAX, 0) if lines are parallel)
	 */
	static FVector LinesIntersection(FVector inFirstLineStart, FVector inFirstLineEnd, FVector inSecondLineStart, FVector inSecondLineEnd);

	/** Get coordinates of lines intersection
	 * @param inFirstLineStart start point of the first line
	 * @param inFirstLineEnd end point of the first line
	 * @param inSecondLineStart start point of the second line
	 * @param inSecondLineEnd end point of the second line
	 * @return bool whether line segments intersect or not
	 */
	static bool DoLineSegmentsIntersect(FVector inFirstLineStart, FVector inFirstLineEnd, FVector inSecondLineStart, FVector inSecondLineEnd, FVector& outIntersection);

	/** Checks whether the point is inside the polygon
	* @param inPolygon polygon to check for the point position
	* @param inPoint point check within the polygon
	* @return bool whether the point is inside the polygon
	*/
	static bool IsPointInPolygon(TArray<FVector>& inPolygon, FVector inPoint);

	/** Get polygon direction as sign
	* @param inPolygon polygon to check for direction
	* @return double negative value (<0) anticlockwise, positive value (>0) if clockwise 
	*/
	static double PolygonDirectionSign(const TArray<FVector>& inPolygon);

	/** Checks whetrer polygon is clockwise or not
	* @param inPolygon polygon to check for direction
	* @return bool true if clockwise, false if anticlockwise 
	*/
	static bool IsPolygonClockwise(const TArray<FVector>& inPolygon);

	/** Check numbers for same signs
	* Zero (0) considred to have a plus (+) sign
	*
	* @param inFirstNumber first number to check for sign
	* @param inSecondNumber second number to check for sign
	* @return bool true if numbers have same sign, otherwise false
	*/
	static bool HasNumbersSameSign(double inFirstNumber, double inSecondNumber);

	/** Get point position relatively to a line looking in direction from line start point to line end point
	* @param inLineStart start point to represent a line
	* @param inLineEnd end point to represent a line
	* @param inPoint point to check for relative position
	* @return double negative value (<0) if point is to the left of a line, positive value (>0) if point is to the right of a line, zero (==0) if point is on a line
	*/
	static double PointToLineRelativePosition(FVector inLineStart, FVector inLineEnd, FVector inPoint);

	/** Get point position relatively to a line looking in direction from line start point to line end point
	* @param inUpperLeftFirst upper-left point of the first rectangle
	* @param inLowerRightFirst lower-right point of the first rectangle
	* @param inUpperLeftSecond upper-left point of the second rectangle
	* @param inLowerRightSecond lower-right point of the second rectangle
	* @return bool true if two rectangles overlap 
	*/
	static bool DoRectanglesOverlap(FVector inUpperLeftFirst, FVector inLowerRightFirst, FVector inUpperLeftSecond, FVector inLowerRightSecond);
	
};

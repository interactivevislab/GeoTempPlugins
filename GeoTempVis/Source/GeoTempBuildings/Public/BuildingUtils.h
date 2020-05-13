#pragma once
#include "BuildingsData.h"
#include "ProceduralMeshComponent.h"
#include "RuntimeMeshComponent.h"
#include "BuildingUtils.generated.h"


/** Struct for storing data of generated mesh section. Use only in building mesh generation related tasks */
USTRUCT()
struct FMeshSectionData
{
    GENERATED_BODY()
    /** Vertex array */
    TArray<FVector> Vertices;
    /** Triangles array */
    TArray<int32> Triangles;
    /** Normals array */
    TArray<FVector> Normals;
    /** UV Coordinates array for main channel */
    TArray<FVector2D> UV0;
    /** UV Coordinates array for addition channel 1 */
    TArray<FVector2D> UV1;
    /** UV Coordinates array for addition channel 2 */
    TArray<FVector2D> UV2;
    /** UV Coordinates array for addition channel 3 */
    TArray<FVector2D> UV3;
    /** Vertex colors array */
    TArray<FLinearColor> VertexColors;
    /** Material to assign to this section */
    UMaterialInterface* Material;
};


/** Struct for storing data of multiple generated mesh sections. Use only in building mesh generation related tasks
 * @see FMeshSectionData */
USTRUCT(BlueprintType)
struct FBuildingMeshData
{
    GENERATED_BODY()

    /** Mesh sections array */
    TArray<FMeshSectionData> Sections;

    /** fn Add section to mesh */
    inline void Append(FBuildingMeshData otherData)
    {
        Sections.Append(otherData.Sections);
    }

    /** Clear all sections */
    inline void Clear()
    {
        Sections.Empty();
    }
};

UINTERFACE(MinimalAPI)
class URoofMaker : public UInterface
{
public:
    GENERATED_BODY()
};

/**Interface for roof mesh generators*/
class GEOTEMPBUILDINGS_API IRoofMaker
{
public:
    GENERATED_BODY()

    /**
     * \fn GenerateRoof
     * \Brief Function that generates roof for building part
     * 
     * @param inBuildingPart building part struct with building data
     * @param inWallMaterial material of wall-type parts of the roof mesh
     * @param inRoofMaterial material of roof-type parts of the roof mesh
     *
     * @return Mesh Data containing roof mesh
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Default")
    FBuildingMeshData GenerateRoof(const FBuildingPart& inBuildingPart, UMaterialInterface* inWallMaterial, UMaterialInterface* inRoofMaterial);
};


/** static function library with helper functions for building mesh construction. Internal use only*/
class MeshHelpers {
    
    /** List of avaliable Roof Makers for use in generation, matched with roof tag values */
    static TMap<FString, TScriptInterface<IRoofMaker>> RoofMakers;
    
public:

    /**
     * \fn GetNeighbourDirections
     * \Brief Routine for obtaining directions to neighboring points on closed contour. Useful in many mesh generation-related algorithms
     * 
     * @param inContour contour in which directions calculated
     * @param inIndex index of contour point for which directions calculated
     * @return Tuple of two vectors in order of the contour (i. e. first argument of pair is actually direction FROM previous point, and second one is direction from current point to next)
     */
    inline static TTuple<FVector, FVector> GetNeighbourDirections(TArray<FVector> inContour, int inIndex)
    {
        int iprev = (inIndex + inContour.Num() - 1)    % inContour.Num();
        int inext = (inIndex + 1)                    % inContour.Num();
        
        auto toPrev = inContour[inIndex] - inContour[iprev];
        auto toNext = inContour[inext] - inContour[inIndex];

        return MakeTuple(toPrev, toNext);
    }

    /**
     * \fn AddRoofMaker
     * \brief Add new Roof Maker for roof generation and assign it to match some value in roof type tag
     * 
     * @param inType value of roof type tag
     * @param inMaker instance of a new Roof Maker
     * @see IRoofMaker
     * @see CheckRoofMaker
     */
    inline static void AddRoofMaker(const FString& inType, TScriptInterface<IRoofMaker> inMaker)
    {
        RoofMakers.Add(inType, inMaker);
    }

    /**
     * \fn CheckRoofMaker
     * \brief Check if there are already exists a Roof Maker for some value in roof type tag
     * 
     * @param inType value of roof type tag     
     * @return Ture if there are already some Roof Marker for that roof type, false otherwise
     * @see AddRoofMaker
     */
    inline static bool CheckRoofMaker(const FString& inType)
    {
        return RoofMakers.Contains(inType);
    }

    /**
     * \fn SetRoofMakers
     * \brief set whole list of roof makers in one function. Removes any previously set matches
     * 
     * @param inRoofMakers Map of matches of roof type tag value and Roof Maker instance
     * @see IRoofMaker
     * @see AddRoofMaker
     */
    inline static void SetRoofMakers(TMap<FString, TScriptInterface<IRoofMaker>> inRoofMakers)
    {
        RoofMakers = inRoofMakers;
    }

    /**
     * \fn CalculateMeshData
     * \brief Create Mesh Data for building part
     * 
     * @param inBuilding building struct for building that owns that part
     * @param inBuildingPart building part struct for current part
     * @param inWallMaterial material for wall-type elements of building mesh
     * @param inRoofMaterial material for roof-type elements of building mesh
     * @param inTriangulationFlags flags used to configure triangulation algorithm for building top and bottom
     * @return Building Mesh Data to use in  
     * @see FBuilding
     * @see FBuildingPart
     */
    static FBuildingMeshData CalculateMeshData(const FBuilding& inBuilding, const FBuildingPart& inBuildingPart, 
        UMaterialInterface* inWallMaterial, UMaterialInterface* inRoofMaterial, const FString& inTriangulationFlags = "");

    /**
     * \fn ConstructProceduralMesh
     * \breif Init Procedural Mesh Component with Mesh Data
     * @param inProcMesh mesh to init
     * @param inMeshData Mesh Data with which mesh will be init
     * @param inFirstSectionIndex initial section of mesh to start writing (in case of append)
     * @see UProceduralMeshComponent
     * @see FBuildingMeshData
     */
    static void ConstructProceduralMesh(UProceduralMeshComponent* inProcMesh, const FBuildingMeshData& inMeshData, int inFirstSectionIndex = 0);

    /**
     * \fn ConstructRuntimeMesh
     * \brief Init Runtime Mesh Component with Mesh Data
     * 
     * @param inMeshComp mesh to init
     * @param inMeshData Mesh Data with which mesh will be init
     * @param inFirstSectionIndex initial section of mesh to start writing (in case of append)
     * @see URuntimeMeshComponent
     * @see FBuildingMeshData
     */
    static void ConstructRuntimeMesh(URuntimeMeshComponent* inMeshComp, const FBuildingMeshData& inMeshData, int inFirstSectionIndex = 0);
};

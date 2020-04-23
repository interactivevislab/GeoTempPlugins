#pragma once

#include "CoreMinimal.h"

#include "GeometryData.h"

#include "PostgisReader.generated.h"


/** Enum for status of Postgre connection. */
UENUM(BlueprintType)
enum class EStatus : uint8
{
	Unconnected			UMETA(DisplayName = "Unconnected"),
	AwaitingConnection	UMETA(DisplayName = "AwaitingConnection"),
	Connected			UMETA(DisplayName = "Connected"),
	PendingOperation	UMETA(DisplayName = "PendingOperation")
};


#pragma warning( disable : 4530)

/**
* \class UPostGisReader
* \brief Class for reading contours data from PostGIS database.
*
* @see FContourData
*/
UCLASS(Config = Game, BlueprintType)
class GEOTEMPPOSTGIS_API UPostGisReader : public UObject
{
	GENERATED_BODY()

public:

	/** Current connection status. */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = Enum)
	EStatus Status = EStatus::Unconnected;

	/** Host to connect. */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Config", Config)
	FString Host;

	/** Port to connect. */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Config", Config)
	FString Port;

	/** Database name to connect. */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Config", Config)
	FString DatabaseName;

	/** User's name in database. */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Config", Config)
	FString User;

	/** User's password in database. */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Config", Config)
	FString Pass;

	/** Error message for log. */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Connection")
	FString Error = TEXT("Everything OK");

	typedef struct pg_conn PGconn;

#if !NOPOSTGRES
	/** Current connection. */
	PGconn* conn = nullptr;
#endif

	/** Initialization flag */
	bool IsInit = false;

	/** Read contours data from WKB entity, converting coordinates to local. */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Connection")
	static FMultipolygonData CreateContourFromBinary(FWkbEntity inEntity, FGeoCoords inGeoCoords);

	/** Read curves from WKB entity, converting coordinates to local. */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Default")
	static FLinesData CreateCurvesFromBinary(FWkbEntity inEntity, FGeoCoords inGeoCoords);

	/** Check and update current connection status. */
	UFUNCTION(BlueprintCallable, Category = "Connection")
	void CheckConnectionStatus(float inDeltaTime);

	/** Load configuration variables. */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Config")
	void LoadConfiguration();

	/** Save configuration variables. */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Config")
	void SaveConfiguration();

	/** Execute request for WKB entities. */
	UFUNCTION(BlueprintCallable, Category = "Default")
	TArray<FWkbEntity> ExecuteRawQuery(FString inQuery, int inGeometryColumnIndex = 0);

	/** Buffer for WKB entities, received through request's execution. */
	TArray<FWkbEntity> RawQueryResult;

	/** Execute request for WKB entities. */
	UFUNCTION(BlueprintCallable, Category = "Default")
	TMap<FString, FWkbEntity> ExecuteIndexedRawQuery(FString inQuery,
		int inKeyColumnIndex = 1, int inGeometryColumnIndex = 0);

	/** Initialize connection */
	UFUNCTION(BlueprintCallable, Category = "Default")
	void InitConnect(bool& outSuccess);
};

#pragma once

#include "CoreMinimal.h"

#include "PosgisData.h"

#include "PostgisReader.generated.h"


UENUM(BlueprintType)
enum class EStatus : uint8
{
	Unconnected			UMETA(DisplayName = "Unconnected"),
	AwaitingConnection	UMETA(DisplayName = "AwaitingConnection"),
	Connected			UMETA(DisplayName = "Connected"),
	PendingOperation	UMETA(DisplayName = "PendingOperation")
};


#pragma warning( disable : 4530)

UCLASS(Config = Game, BlueprintType)
class GEOTEMPPOSTGIS_API APostgisReader : public AActor
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = Enum)
	EStatus Status = EStatus::Unconnected;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Config", Config)
	FString Host;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Config", Config)
	FString Port;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Config", Config)
	FString DatabaseName;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Config", Config)
	FString User;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Config", Config)
	FString Pass;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Connection")
	FString Error = TEXT("Everything OK");

	typedef struct pg_conn PGconn;

#if !NOPOSTGRES
	PGconn* conn = nullptr;
#endif

	bool IsInit = false;

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Connection")
	static FPosgisContourData CreateContourFromBinary(FPostGisBinaryEntity inEntity, FGeoCoords inGeoCoords);

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Default")
	static FPosgisLinesData CreateCurvesFromBinary(FPostGisBinaryEntity inEntity, FGeoCoords inGeoCoords);

	UFUNCTION(BlueprintCallable, Category = "Connection")
	void CheckConnectionStatus(float inDeltaTime);

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Config")
	void LoadConfiguration();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Config")
	void SaveConfiguration();

	UFUNCTION(BlueprintCallable, Category = "Default")
	TArray<FPostGisBinaryEntity> ExecuteRawQuery(FString inQuery, int inGeometryColumnIndex = 0);

	UFUNCTION(BlueprintCallable, Category = "Default")
	TMap<FString, FPostGisBinaryEntity> ExecuteIndexedRawQuery(FString inQuery, 
		int inKeyColumnIndex = 1, int inGeometryColumnIndex = 0);

	UFUNCTION(BlueprintCallable, Category = "Default")
	void InitConnect(bool& outSuccess);
};

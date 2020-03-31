#pragma once

#include "CoreMinimal.h"
#include "PosgisData.h"
#include "PostgisLoader.generated.h"

UENUM(BlueprintType)
enum class EStatus : uint8 {
	Unconnected, AwaitingConnection, Connected, PendingOperation,
};

USTRUCT(BlueprintType)
struct FPostGisBinaryEntity
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Default")
	TArray<uint8> Geometry;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Default")
	TMap<FString, FString> Tags;
};

#pragma warning( disable : 4530)
UCLASS(Config = Game, BlueprintType)
class GEOTEMPPOSTGIS_API APostgisReader : public AActor
{
	GENERATED_BODY()

public:

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;;

	virtual void BeginDestroy() override;
	
	UFUNCTION(BlueprintCallable, CallInEditor, Category="Connection")
		static FPosgisContourData CreateContourFromBinary(FPostGisBinaryEntity entity,
			ProjectionType projection, float originLon, float originLat);


	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Default")
		static FPosgisLinesData CreateCurvesFromBinary(FPostGisBinaryEntity entity,
			ProjectionType projection, float originLon, float originLat);

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

	UFUNCTION(BlueprintCallable, Category = "Connection")
		void CheckConnectionStatus(float DeltaTime);

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Config")
		void LoadConfiguration();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Config")
		void SaveConfiguration();
	typedef struct pg_conn PGconn;
#if !NOPOSTGRES
	PGconn* conn = NULL;
#endif

	bool IsInit = false;

	UFUNCTION(BlueprintCallable, Category = "Default")
		TArray<FPostGisBinaryEntity> ExecuteRawQuery(FString query, int geometryColumnIndex = 0);

	UFUNCTION(BlueprintCallable, Category = "Default")
		TMap<FString, FPostGisBinaryEntity> ExecuteIndexedRawQuery(FString query, int keyColumnIndex = 1, int geometryColumnIndex = 0);

	UFUNCTION(BlueprintCallable, Category = "Default")
		void InitConnect(bool& sucess);
};
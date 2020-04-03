#pragma once

#include "CoreMinimal.h"

#include "HttpClient.h"
#include "Tickable.h"

#include "OsmManager.generated.h"


class UHttpRequest;


UCLASS(BlueprintType, Config = Game)
class GEOTEMPOSM_API UOsmManager : public UObject, public FTickableGameObject
{
	GENERATED_BODY()

public:

#pragma region Requests

	UFUNCTION(BlueprintCallable) 
	UHttpRequest* GetOsmDataForRelation(FString inRelationId);

	UFUNCTION(BlueprintCallable)
	UHttpRequest* GetOsmDataForBoundingBox(float inLeft, float inBottom, float inRight, float inTop);

#pragma endregion

	UFUNCTION(BlueprintCallable)
	void Init(FString inApiVersion = "0.6");

protected:

	UPROPERTY()
	UHttpClient* client;

private:

	using ParametersSet = UHttpClient::ParametersSet;
	using Parameter		= UHttpClient::Parameter;

	static const FString POST_VERB;
	static const FString GET_VERB;

	FString apiVersion;

	UHttpRequest* CreateRequest(FString inVerb, FString inUrlTail, ParametersSet inParameters, FString inContent);
	UHttpRequest* CreateRequest(FString inVerb, FString inUrlTail, ParametersSet inParameters);
	UHttpRequest* CreateRequest(FString inVerb, FString inUrlTail, FString inContent);
	UHttpRequest* CreateRequest(FString inVerb, FString inUrlTail);

#pragma region Requests deleting

	int		nextRequestId			= 0;
	float	accumulatedDeltaTime	= 0;
	float	deletingFrequency		= 1;

	UPROPERTY()
	TMap<int, UHttpRequest*> currentRequests;
	
	UPROPERTY()
	TArray<int> finishedRequestsIds;
	
	UFUNCTION()
	void MarkRequestForDelete(int inId);
	void DeleteFinishedRequests();

#pragma endregion

#pragma region FTickableGameObject

	bool tickEnabled = false;

	void Tick(float inDeltaTime) override;
	bool IsTickable() const override;
	bool IsTickableInEditor() const override;
	bool IsTickableWhenPaused() const override;
	TStatId GetStatId() const override;

#pragma endregion

};

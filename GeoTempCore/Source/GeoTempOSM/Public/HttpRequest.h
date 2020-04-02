#pragma once

#include "CoreMinimal.h"

#include "Runtime/Online/HTTP/Public/Http.h"

#include "OsmManager.h"

#include "HttpRequest.generated.h"


UCLASS(BlueprintType)
class GEOTEMPOSM_API UHttpRequest : public UObject
{
	GENERATED_BODY()

public:

	TSharedPtr<IHttpRequest> request;
	UOsmManager * Manager;
	int Id;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRequestComleted, FString, inResponseString);

	UPROPERTY(BlueprintAssignable)
	FOnRequestComleted OnCompleted;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReadyToDelete, int, inId);

	FOnReadyToDelete OnReadyToDelete;

	UHttpRequest();

	UFUNCTION(BlueprintCallable)
	void StartRequest();

	void OnHttpRequestCompleted(FHttpRequestPtr inRequest, FHttpResponsePtr inResponse, bool inSucceeded);
};
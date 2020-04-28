#pragma once

#include "CoreMinimal.h"

#include "Runtime/Online/HTTP/Public/Http.h"

#include "HttpRequest.generated.h"


class UOsmManager;


/**
* \class UHttpRequest
* \brief Class representing HTTP request.
*
* @see UOsmManager
*/
UCLASS(BlueprintType)
class GEOTEMPOSM_API UHttpRequest : public UObject
{
	GENERATED_BODY()

public:

	UHttpRequest();

	/** Initialize with inner HTTP request. */
	void Init(TSharedPtr<IHttpRequest> inRequest);

	/** Returns ID. */
	int GetId();

	/** Delegate type for providing responses. */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRequestComleted, FString, inResponseString);

	/** Delegate for providing responses. */
	UPROPERTY(BlueprintAssignable)
	FOnRequestComleted OnCompleted;

	/** Delegate type for indicating readiness for deletion. */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReadyToDelete, int, inId);

	/** Delegate for indicating readiness for deletion. */
	FOnReadyToDelete OnReadyToDelete;

	/** Execute request. */
	UFUNCTION(BlueprintCallable)
	void StartRequest();

	/** Process request results. */
	void OnHttpRequestCompleted(FHttpRequestPtr inRequest, FHttpResponsePtr inResponse, bool inSucceeded);

private:

	/** Request ID. */
	int id;

	/** Next unique ID. */
	static int nextId;

	/** Inner HTTP request. */
	TSharedPtr<IHttpRequest> request;
};

#pragma once

#include "CoreMinimal.h"

#include "HttpClient.h"
#include "Tickable.h"

#include "OsmManager.generated.h"


class UHttpRequest;


/**
* \class UOsmManager
* \brief Class for creating specialized requests.
*
* @see UOsmManager
*/
UCLASS(BlueprintType, Config = Game)
class GEOTEMPOSM_API UOsmManager : public UObject, public FTickableGameObject
{
	GENERATED_BODY()

public:

#pragma region Requests

	/** Creates request for loading OSM data for relation. */
	UFUNCTION(BlueprintCallable) 
	UHttpRequest* GetOsmDataForRelation(FString inRelationId);

	/** Creates request for loading OSM data for bounding box. */
	UFUNCTION(BlueprintCallable)
	UHttpRequest* GetOsmDataForBoundingBox(float inLeft, float inBottom, float inRight, float inTop);

	/** Creates request for loading OSM data for IDs. */
	UFUNCTION(BlueprintCallable)
	UHttpRequest* GetOsmDataForIds(TSet<int> inIDs);

	/** Creates request for loading OSM data for relation. */
	UFUNCTION(BlueprintCallable)
	UHttpRequest* GetFullOsmDataForRelation(FString inRelationId);

#pragma endregion

	/** Initialize manager. */
	UFUNCTION(BlueprintCallable)
	void Init(FString inOsmApiVersion = "0.6");

protected:

	/** HTTP client. */
	UPROPERTY()
	UHttpClient* client;

private:

	/** URL parameters set. */
	using ParametersSet = UHttpClient::ParametersSet;

	/** URL parameter. */
	using Parameter		= UHttpClient::Parameter;

	/** "POST" string for requests. */
	static const FString POST_VERB;

	/** "GET" string for requests. */
	static const FString GET_VERB;

	/** Version of OSM API for requests. */
	FString apiVersion;

	/**
	* \fn CreateRequest
	* \brief Create HTTP request.
	*
	* @param inVerb			Verb of request (POST, GET...)
	* @param inUrlTail		Main part of request.
	* @param inParameters	Request parameters.
	* @param inContent		Request content.
	*/
	UHttpRequest* CreateRequest(FString inVerb, FString inUrlTail, ParametersSet inParameters, FString inContent);

	/**
	* \fn CreateRequest
	* \brief Create HTTP request.
	*
	* @param inVerb			Verb of request (POST, GET...)
	* @param inUrlTail		Main part of request.
	* @param inParameters	Request parameters.
	*/
	UHttpRequest* CreateRequest(FString inVerb, FString inUrlTail, ParametersSet inParameters);

	/**
	* \fn CreateRequest
	* \brief Create HTTP request.
	*
	* @param inVerb		Verb of request (POST, GET...)
	* @param inUrlTail	Main part of request.
	* @param inContent	Request content.
	*/
	UHttpRequest* CreateRequest(FString inVerb, FString inUrlTail, FString inContent);

	/**
	* \fn CreateRequest
	* \brief Create HTTP request.
	*
	* @param inVerb		Verb of request (POST, GET...)
	* @param inUrlTail	Main part of request.
	*/
	UHttpRequest* CreateRequest(FString inVerb, FString inUrlTail);

#pragma region Requests deleting

	/** Elapsed time from previous deletion. */
	float	accumulatedDeltaTime	= 0;

	/** Period in seconds for request deleting. */
	float	deletingPeriod			= 1;

	/** Current requests, mapped by IDs. */
	UPROPERTY()
	TMap<int, UHttpRequest*> currentRequests;
	
	/** Requests, marked for deletion. */
	UPROPERTY()
	TArray<int> finishedRequestsIds;
	
	/** Mark request for deletion by ID. */
	UFUNCTION()
	void MarkRequestForDelete(int inId);

	/** Deletes finished requests. */
	void DeleteFinishedRequests();

#pragma endregion

#pragma region FTickableGameObject

	/** Flag for tick enabling. */
	bool tickEnabled = false;

	//~ Begin FTickableGameObject overriding

	void Tick(float inDeltaTime) override;
	bool IsTickable() const override;
	bool IsTickableInEditor() const override;
	bool IsTickableWhenPaused() const override;
	TStatId GetStatId() const override;

	//~ Begin FTickableGameObject overriding

#pragma endregion

};

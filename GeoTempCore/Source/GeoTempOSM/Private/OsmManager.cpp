#include "OsmManager.h"
#include "HttpRequest.h"


//example: UrlWithArgs("/user/{0}", login)
#define UrlWithArgs(formatString, ...) FString::Format(TEXT(formatString), TArray<FStringFormatArg>{ ##__VA_ARGS__ })


void UOsmManager::Init(FString inOsmApiVersion)
{
	apiVersion = inOsmApiVersion;
	tickEnabled = true;
	client = NewObject<UHttpClient>(this);
	client->Init();
	SaveConfig();
}


UHttpRequest* UOsmManager::CreateRequest(FString inVerb, FString inUrlTail, 
	ParametersSet inParameters, FString inContent)
{
	auto* apiRequest = NewObject<UHttpRequest>(this);
	auto innerRequest = client->CreateRequest(apiVersion, inVerb, inUrlTail, inParameters, inContent);
	apiRequest->Init(innerRequest);

	apiRequest->OnReadyToDelete.AddDynamic(this, &UOsmManager::MarkRequestForDelete);

	currentRequests.Add(apiRequest->GetId(), apiRequest);

	return apiRequest;
}


UHttpRequest* UOsmManager::CreateRequest(FString inVerb, FString inUrlTail, ParametersSet inParameters)
{
	return CreateRequest(inVerb, inUrlTail, inParameters, "");
}


UHttpRequest* UOsmManager::CreateRequest(FString inVerb, FString inUrlTail, FString inContent)
{
	return CreateRequest(inVerb, inUrlTail, ParametersSet(), inContent);
}


UHttpRequest* UOsmManager::CreateRequest(FString inVerb, FString inUrlTail)
{
	return CreateRequest(inVerb, inUrlTail, ParametersSet(), "");
}


#pragma region FTickableGameObject

void UOsmManager::Tick(float inDeltaTime)
{
	accumulatedDeltaTime += inDeltaTime;
	if (accumulatedDeltaTime > deletingPeriod)
	{
		accumulatedDeltaTime -= deletingPeriod;
		DeleteFinishedRequests();
	}
}


bool UOsmManager::IsTickable() const
{
	return tickEnabled;
}


bool UOsmManager::IsTickableInEditor() const
{
	return false;
}


bool UOsmManager::IsTickableWhenPaused() const
{
	return tickEnabled;
}


TStatId UOsmManager::GetStatId() const
{
	return TStatId();
}


#pragma endregion


void UOsmManager::MarkRequestForDelete(int inId)
{
	finishedRequestsIds.Add(inId);
}


void UOsmManager::DeleteFinishedRequests()
{
	for (auto id : finishedRequestsIds)
	{
		auto request = *(currentRequests.Find(id));
		request->OnReadyToDelete.RemoveAll(this);
		currentRequests.Remove(id);
	}
	finishedRequestsIds.Empty();
}


const FString UOsmManager::POST_VERB	= "POST";
const FString UOsmManager::GET_VERB		= "GET";


#pragma region Requests

UHttpRequest* UOsmManager::GetOsmDataForRelation(FString inRelationId)
{
	return CreateRequest(GET_VERB, UrlWithArgs("/relation/{0}", inRelationId));
}


UHttpRequest* UOsmManager::GetOsmDataForBoundingBox(float inLeft, float inBottom, float inRight, float inTop)
{
	return CreateRequest(GET_VERB, "/map", ParametersSet{
		Parameter {"bbox", UrlWithArgs("{0},{1},{2},{3}", inLeft, inBottom, inRight, inTop)}
	});
}

#pragma endregion

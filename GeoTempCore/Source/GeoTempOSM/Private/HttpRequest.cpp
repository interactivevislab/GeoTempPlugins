#include "HttpRequest.h"


UHttpRequest::UHttpRequest()
{
}


void UHttpRequest::StartRequest()
{
	if (auto t = request.Get())
	{
		t->ProcessRequest();
	}
}


void UHttpRequest::OnHttpRequestCompleted(FHttpRequestPtr inRequest, FHttpResponsePtr inResponse, bool inSucceeded)
{
	FString content = inResponse->GetContentAsString();
	OnCompleted.Broadcast(content);

	OnReadyToDelete.Broadcast(Id);
}

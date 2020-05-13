#include "HttpRequest.h"


int UHttpRequest::nextId = 0;


UHttpRequest::UHttpRequest() : id(nextId++)
{
}


int UHttpRequest::GetId()
{
    return id;
}


void UHttpRequest::Init(TSharedPtr<IHttpRequest> inRequest)
{
    request = inRequest;
    request->OnProcessRequestComplete().BindLambda(
        [this](FHttpRequestPtr inHttpRequest, FHttpResponsePtr inHttpResponse, bool inSucceeded)
    {
        OnHttpRequestCompleted(inHttpRequest, inHttpResponse, inSucceeded);
    });
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
    OnCompleted.Broadcast(inResponse->GetContentAsString());
    OnReadyToDelete.Broadcast(id);
    request->OnProcessRequestComplete().Unbind();
}

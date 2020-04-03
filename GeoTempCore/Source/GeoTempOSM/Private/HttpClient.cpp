#include "HttpClient.h"


UHttpClient::UHttpClient(const FObjectInitializer& inObjectInitializer) : Super(inObjectInitializer)
{
}


void UHttpClient::Init()
{
	httpModule = &FHttpModule::Get();
	SaveConfig();
}


TSharedPtr<IHttpRequest> UHttpClient::CreateRequest(FString inApiVersion, FString inVerb, FString inUrlTail, 
	ParametersSet inParameters, FString inContent)
{
	TSharedRef<IHttpRequest> request = httpModule->CreateRequest();

	FString url = httpRequestHead + inApiVersion + inUrlTail;

	if (inParameters.Num() > 0)
	{
		url += '?';
		for (auto parameter : inParameters)
		{
			url += parameter.Key + '=' + parameter.Value + '&';
		}
		url.RemoveFromEnd("&");
	}

	request->SetURL(url);
	request->SetVerb(inVerb);
	request->SetHeader("User-Agent", "UnrealEngine-Agent");

	if (!inContent.IsEmpty())
	{
		request->SetHeader("Content-Type", "text/xml");
		request->SetContentAsString(inContent);
	}
	
	return request;
}

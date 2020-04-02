#pragma once

#include "CoreMinimal.h"

#include "Runtime/Online/HTTP/Public/Http.h"

#include "HttpClient.generated.h"


UCLASS(Config = Game)
class GEOTEMPOSM_API UHttpClient : public UObject
{
	GENERATED_BODY()

public:	

	UHttpClient(const FObjectInitializer& inObjectInitializer);

	using Parameter		= TPair<FString, FString>;
	using ParametersSet = TSet<Parameter>;

	TSharedPtr<IHttpRequest> CreateRequest(FString apiVersion, FString verb, FString urlTail, 
		ParametersSet parameters, FString content);

	void Init();

private:
	FHttpModule* httpModule;

	UPROPERTY(Config)
	FString httpRequestHead = "https://api.openstreetmap.org/api/";
};

#pragma once

#include "CoreMinimal.h"

#include "Http.h"

#include "HttpClient.generated.h"


/**
* \class UHttpClient
* \brief Class for constructing HTTP requests for OSM.
*
* @see UOsmManager
*/
UCLASS(Config = Game)
class GEOTEMPOSM_API UHttpClient : public UObject
{
    GENERATED_BODY()

public:    

    UHttpClient(const FObjectInitializer& inObjectInitializer);

    /** URL parameter. */
    using Parameter        = TPair<FString, FString>;

    /** URL parameter set. */
    using ParametersSet = TSet<Parameter>;

    /**
    * \fn CreateRequest
    * \brief Create HTTP request.
    *
    * @param apiVersion    OSM API version
    * @param verb        Verb of request (POST, GET...)
    * @param urlTail    Main part of request.
    * @param parameters    Request parameters.
    * @param content    Request content.
    */
    TSharedPtr<IHttpRequest> CreateRequest(FString apiVersion, FString verb, FString urlTail, 
        ParametersSet parameters, FString content);

    /** Initialize client. */
    void Init();

private:

    /**Module for Http request implementations */
    FHttpModule* httpModule;

    /** Head of HTTP request. */
    UPROPERTY(Config)
    FString httpRequestHead = "https://api.openstreetmap.org/api/";
};

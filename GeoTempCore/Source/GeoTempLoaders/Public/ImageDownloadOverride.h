// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
//Override of Epic Class to obtain raw texture data alongside the texture itself

#pragma once

#include "CoreMinimal.h"
#include "Engine/Texture2DDynamic.h"
#include "UObject/ObjectMacros.h"
#include "Interfaces/IHttpRequest.h"
#include "Kismet/BlueprintAsyncActionBase.h"

#include "ImageDownloadOverride.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDownloadImageDelegateExtended, UTexture2DDynamic*, Texture, TArray<uint8>, RawData);

UCLASS()
class GEOTEMPLOADERS_API UImageDownloadOverride : public UBlueprintAsyncActionBase
{
    GENERATED_UCLASS_BODY()

public:
    UFUNCTION(BlueprintCallable, meta=( BlueprintInternalUseOnly="true" ))
    static UImageDownloadOverride* DownloadImage(FString URL);

    UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
    static UTexture2DDynamic* LoadImageFromRaw(TArray<FLinearColor>& inData, int inResolution);

public:

    UPROPERTY(BlueprintAssignable)
    FDownloadImageDelegateExtended OnSuccess;

    UPROPERTY(BlueprintAssignable)
    FDownloadImageDelegateExtended OnFail;

public:

    void Start(FString URL);

	

private:

    /** Handles image requests coming from the web */
    void HandleImageRequest(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded);
};

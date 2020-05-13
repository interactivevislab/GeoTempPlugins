#include "TextureDownloader.h"

#include "UrlSourceTilePreparer.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "HAL/PlatformFilemanager.h"

void UTextureDownloader::StartDownloadingTile(FTileCoordinates inTileCoords, const FString& inUrl, FString inCacheFolder, FString inCacheFile)
{
    if (inCacheFile.IsEmpty())
    {
        inCacheFile = FString::Format(TEXT("{0}-{1}-{2}__{3}.cache"), TArray<FStringFormatArg>{inTileCoords.X, inTileCoords.Y, inTileCoords.Z, Channel});
    }
    CachePath = inCacheFolder;
    CacheFile = inCacheFile;

    if (CheckCache())
    {
        TextureCoords = inTileCoords;    
        LoadCache();
    }
    else
    {
        TextureCoords = inTileCoords;    
        Loader = UImageDownloadOverride::DownloadImage(inUrl);
        
        Loader->OnSuccess.AddDynamic(this, &UTextureDownloader::OnTextureLoadedWeb);
        Loader->OnFail.AddDynamic(this, &UTextureDownloader::OnLoadFailed);
    }
}

bool UTextureDownloader::CheckCache()
{
    auto& pf = FPlatformFileManager::Get().GetPlatformFile();
    if (pf.CreateDirectoryTree(*CachePath))
    {
        return pf.FileExists(*(CachePath + "/" + CacheFile));
    }
    return false;
}

void UTextureDownloader::LoadCache()
{
    auto filePath = CachePath + "/" + CacheFile;
    auto& pf = FPlatformFileManager::Get().GetPlatformFile();

    //auto FileHandle = pf.OpenRead(*filePath);
    /*FileHandle->Read(textureInMemory.GetData(), Resolution * Resolution * 4);
    OnTextureLoadedFromDisk();*/
    
    auto FileHandle = pf.OpenAsyncRead(*filePath);	
    ReadCallbackFunction = [this](bool bWasCancelled, IAsyncReadRequest* Request)
    {
        OnTextureLoadedFromDisk();   	
    };
    
    textureInMemory.SetNum(Resolution * Resolution * 4);
    

    
    FileHandle->ReadRequest(0, Resolution * Resolution * 4, AIOP_Normal, &ReadCallbackFunction, textureInMemory.GetData());
}

void UTextureDownloader::WriteCache()
{
    auto filePath = CachePath + "/" + CacheFile;
    auto& pf = FPlatformFileManager::Get().GetPlatformFile();
    auto FileHandle = pf.OpenWrite(*filePath);
    if (!FileHandle) return; //sad	
    FileHandle->Write(textureInMemory.GetData(), textureInMemory.Num());
    FileHandle->Flush();	
}

void UTextureDownloader::OnTextureLoadedWeb(UTexture2DDynamic* inTexture, TArray<uint8> inData)
{
    if (!inTexture->IsValidLowLevel())
    {
        TilePreparer->FreeLoader(TextureCoords);
        UE_LOG(LogTemp, Warning, TEXT("Loaded texture is corrupt"));
        return;
    }
    if(!Material->IsValidLowLevel())
    {
        TilePreparer->FreeLoader(TextureCoords);
        UE_LOG(LogTemp, Warning, TEXT("Texture loaded for already destroyed tile"));
        return;
    }
    Material->SetTextureParameterValue(FName(*Channel), inTexture);    
    textureInMemory = inData;
    WriteCache();
    TileContainer->CacheTexture(TextureCoords, inTexture, Channel, textureInMemory);
    
    
}

void UTextureDownloader::OnTextureLoadedFromDisk()
{
    TilePreparer->QueueLoader(this);    
}

void UTextureDownloader::OnTextureLoadedFromDiskMainThread()
{
    if (UTexture2DDynamic* Texture = UTexture2DDynamic::Create(Resolution, Resolution))
    {
        Texture->SRGB = true;
        Texture->UpdateResource();

        FTexture2DDynamicResource* TextureResource = static_cast<FTexture2DDynamicResource*>(Texture->Resource);

        ENQUEUE_RENDER_COMMAND(FWriteRawDataToTexture)(
            [&, TextureResource](FRHICommandListImmediate& RHICmdList)
        {
            check(IsInRenderingThread());
            uint8* dataPtr = textureInMemory.GetData();
            FRHITexture2D* TextureRHI = TextureResource->GetTexture2DRHI();

            int32 Width = TextureRHI->GetSizeX();
            int32 Height = TextureRHI->GetSizeY();

            uint32 DestStride = 0;
            uint8* DestData = reinterpret_cast<uint8*>(RHILockTexture2D(TextureRHI, 0, RLM_WriteOnly, DestStride, false, false));

            for (int32 y = 0; y < Height; y++)
            {
                uint8* DestPtr = &DestData[(Height - 1 - y) * DestStride];

                const FColor* SrcPtr = &((FColor*)dataPtr)[(Resolution - 1 - y) * Resolution];
                for (int32 x = 0; x < Width; x++)
                {
                    *DestPtr++ = SrcPtr->B;
                    *DestPtr++ = SrcPtr->G;
                    *DestPtr++ = SrcPtr->R;
                    *DestPtr++ = SrcPtr->A;
                    SrcPtr++;
                }
            }

            RHIUnlockTexture2D(TextureRHI, 0, false, false);
        });

        if (!Texture->IsValidLowLevel())
        {
            TilePreparer->FreeLoader(TextureCoords);
            UE_LOG(LogTemp, Warning, TEXT("Loaded texture is corrupt"));
            return;
        }
        if (!Material->IsValidLowLevel())
        {
            TilePreparer->FreeLoader(TextureCoords);
            UE_LOG(LogTemp, Warning, TEXT("Texture loaded for already destroyed tile"));
            return;
        }
        Material->SetTextureParameterValue(FName(*Channel), Texture);
        TileContainer->CacheTexture(TextureCoords, Texture, Channel, textureInMemory);
    }
}

void UTextureDownloader::OnLoadFailed(UTexture2DDynamic* inTexture, TArray<uint8> inData)
{
    TilePreparer->FreeLoader(TextureCoords);
    UE_LOG(LogTemp, Warning, TEXT("Load failed"));
}

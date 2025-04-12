
#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Http.h"
#include "GoogleCloudAPI.generated.h"

// This declares the delegate type for callbacks
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnGoogleCloudResponse, bool, Success, FString, ResponseContent);

UCLASS()
class ESCAPE_API UGoogleCloudAPI : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Google Cloud")
    static void CallGoogleCloudAPI(const FString& Endpoint, const FString& Method, const FString& Payload, const FString& ApiKey, const FOnGoogleCloudResponse& OnComplete);

private:
    static void HandleResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully, FOnGoogleCloudResponse OnComplete);
};
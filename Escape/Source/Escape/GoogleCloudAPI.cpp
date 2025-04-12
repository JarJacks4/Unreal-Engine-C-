#include "GoogleCloudAPI.h"
#include "Json.h"
#include "Engine.h"
#include "JsonObjectConverter.h"

void UGoogleCloudAPI::CallGoogleCloudAPI(const FString& Endpoint, const FString& Method, const FString& Payload, const FString& ApiKey, const FOnGoogleCloudResponse& OnComplete)
{
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();

    // Set the URL (base URL + endpoint)
    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Calling Google Cloud API..."));
    FString URL = "https://escape-ujuzxr-334104837337.us-central1.run.app/" + Endpoint;
    Request->SetURL(URL);

    // Set the verb (GET, POST, etc)
    Request->SetVerb(Method);

    // Set headers
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *ApiKey));

    // Set content if it's a POST or PUT request
    if (!Payload.IsEmpty())
    {
        Request->SetContentAsString(Payload);
    }

    // Set the callback for when the request completes
    Request->OnProcessRequestComplete().BindStatic(&UGoogleCloudAPI::HandleResponse, OnComplete);

    // Submit the request
    Request->ProcessRequest();
}

void UGoogleCloudAPI::HandleResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully, FOnGoogleCloudResponse OnComplete)
{
    if (bConnectedSuccessfully && Response.IsValid())
    {
        OnComplete.ExecuteIfBound(true, Response->GetContentAsString());
    }
    else
    {
        OnComplete.ExecuteIfBound(false, TEXT("Request failed"));
    }
}
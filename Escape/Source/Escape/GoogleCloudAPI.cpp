// GoogleCloudAPI.cpp
#include "GoogleCloudAPI.h"

const FString UGoogleCloudAPI::ApiKey = "AIzaSyDeYSBzP8-2l1-o9L1kA0gO8v9xU9K7lUk";
const FString UGoogleCloudAPI::ServerUrl = "https://escape-ujuzxr-334104837337.us-central1.run.app";

void UGoogleCloudAPI::SendChatMessage(const FString& Message, const FString& SessionId,
    const FOnChatResponseReceived& OnSuccess,
    const FOnChatError& OnError)
{
    // Create HTTP request
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
    HttpRequest->SetVerb("POST");
    HttpRequest->SetURL(ServerUrl + "/chat");
    HttpRequest->SetHeader("Content-Type", "application/json");
    HttpRequest->SetHeader("Authorization", "Bearer " + ApiKey);

    // Create request body
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
    JsonObject->SetStringField("message", Message);
    JsonObject->SetStringField("session_id", SessionId);

    FString RequestBodyString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBodyString);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

    HttpRequest->SetContentAsString(RequestBodyString);

    // Set completion callback
    HttpRequest->OnProcessRequestComplete().BindLambda([OnSuccess, OnError](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
        {
            if (!bWasSuccessful || !Response.IsValid())
            {
                OnError.ExecuteIfBound("Failed to connect to server");
                return;
            }

            if (Response->GetResponseCode() != 200)
            {
                OnError.ExecuteIfBound(FString::Printf(TEXT("Server returned error code: %d"), Response->GetResponseCode()));
                return;
            }

            // Parse the response
            TSharedPtr<FJsonObject> JsonObject;
            TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
            if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
            {
                OnError.ExecuteIfBound("Failed to parse response");
                return;
            }

            // Create the response struct
            FChatResponse ChatResponse;
            ChatResponse.SessionId = JsonObject->GetStringField("session_id");
            ChatResponse.Response = JsonObject->GetStringField("response");

            // Parse conversation array
            TArray<TSharedPtr<FJsonValue>> ConversationArray = JsonObject->GetArrayField("conversation");
            for (const TSharedPtr<FJsonValue>& Value : ConversationArray)
            {
                ChatResponse.Conversation.Add(Value->AsString());
            }

            OnSuccess.ExecuteIfBound(ChatResponse);
        });

    // Send request
    HttpRequest->ProcessRequest();
}

void UGoogleCloudAPI::CreateChatSession(const FOnSessionCreated& OnSuccess,
    const FOnChatError& OnError)
{
    // Create HTTP request
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
    HttpRequest->SetVerb("GET");
    HttpRequest->SetURL(ServerUrl + "/");
    HttpRequest->SetHeader("Authorization", "Bearer " + ApiKey);

    // Set completion callback
    HttpRequest->OnProcessRequestComplete().BindLambda([OnSuccess, OnError](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
        {
            if (!bWasSuccessful || !Response.IsValid())
            {
                OnError.ExecuteIfBound("Failed to connect to server");
                return;
            }

            if (Response->GetResponseCode() != 200)
            {
                OnError.ExecuteIfBound(FString::Printf(TEXT("Server returned error code: %d"), Response->GetResponseCode()));
                return;
            }

            // Parse the response
            TSharedPtr<FJsonObject> JsonObject;
            TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
            if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
            {
                OnError.ExecuteIfBound("Failed to parse response");
                return;
            }

            // Extract session ID
            FString SessionId = JsonObject->GetStringField("session_id");
            OnSuccess.ExecuteIfBound(SessionId);
        });

    // Send request
    HttpRequest->ProcessRequest();
}
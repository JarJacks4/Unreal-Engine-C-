// GoogleCloudAPI.cpp
#include "GoogleCloudAPI.h"

#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

const FString UGoogleCloudAPI::ServerUrl = TEXT("https://lucillellm-334104837337.us-central1.run.app");
FString UGoogleCloudAPI::SessionCookie;

static void LogHttpResponse(const TCHAR* Prefix, const TSharedPtr<IHttpResponse, ESPMode::ThreadSafe>& Response)
{
    if (!Response.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("%s: <no response>"), Prefix);
        return;
    }
    UE_LOG(LogTemp, Warning, TEXT("%s: HTTP %d | %s"), Prefix, Response->GetResponseCode(), *Response->GetContentAsString());
}

void UGoogleCloudAPI::CreateChatSession(const FOnSessionCreated& OnSuccess,
    const FOnChatError& OnError)
{
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
    HttpRequest->SetVerb(TEXT("GET"));
    // IMPORTANT: trailing slash — this is the GET / endpoint
    HttpRequest->SetURL(ServerUrl + TEXT("/"));
    HttpRequest->SetHeader(TEXT("Accept"), TEXT("application/json"));

    HttpRequest->OnProcessRequestComplete().BindLambda(
        [OnSuccess, OnError](FHttpRequestPtr /*Request*/, FHttpResponsePtr Response, bool bWasSuccessful)
        {
            if (!bWasSuccessful || !Response.IsValid())
            {
                OnError.ExecuteIfBound(TEXT("Failed to connect to server"));
                return;
            }

            LogHttpResponse(TEXT("GET /"), Response);

            if (Response->GetResponseCode() != 200)
            {
                OnError.ExecuteIfBound(FString::Printf(TEXT("HTTP %d — %s"),
                    Response->GetResponseCode(), *Response->GetContentAsString()));
                return;
            }

            // Capture Set-Cookie (optional; some backends read either cookie or body)
            const FString SetCookie = Response->GetHeader(TEXT("Set-Cookie"));
            if (!SetCookie.IsEmpty())
            {
                FString CookieLine = SetCookie;
                int32 SemiIndex = INDEX_NONE;
                if (CookieLine.FindChar(TEXT(';'), SemiIndex))
                {
                    CookieLine = CookieLine.Left(SemiIndex); // keep "name=value"
                }
                UGoogleCloudAPI::SessionCookie = CookieLine;
                UE_LOG(LogTemp, Warning, TEXT("Captured Cookie: %s"), *UGoogleCloudAPI::SessionCookie);
            }

            // Parse JSON
            const FString Raw = Response->GetContentAsString();
            TSharedPtr<FJsonObject> Json;
            const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Raw);
            if (!FJsonSerializer::Deserialize(Reader, Json) || !Json.IsValid())
            {
                OnError.ExecuteIfBound(TEXT("Failed to parse response JSON"));
                return;
            }

            FString SessionId;
            if (!Json->TryGetStringField(TEXT("session_id"), SessionId) || SessionId.IsEmpty())
            {
                OnError.ExecuteIfBound(TEXT("No session_id field in response"));
                return;
            }

            OnSuccess.ExecuteIfBound(SessionId);
        });

    HttpRequest->ProcessRequest();
}

void UGoogleCloudAPI::SendChatMessage(const FString& Message,
    const FString& SessionId,
    const FOnChatResponseReceived& OnSuccess,
    const FOnChatError& OnError)
{
    if (SessionId.IsEmpty())
    {
        OnError.ExecuteIfBound(TEXT("SessionId is empty — call CreateChatSession first."));
        return;
    }
    if (Message.IsEmpty())
    {
        OnError.ExecuteIfBound(TEXT("Message is empty."));
        return;
    }

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
    HttpRequest->SetVerb(TEXT("POST"));
    HttpRequest->SetURL(ServerUrl + TEXT("/chat"));
    HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    HttpRequest->SetHeader(TEXT("Accept"), TEXT("application/json"));

    // (Optional) send cookie captured from GET /
    if (!UGoogleCloudAPI::SessionCookie.IsEmpty())
    {
        HttpRequest->SetHeader(TEXT("Cookie"), UGoogleCloudAPI::SessionCookie);
    }

    // JSON body
    TSharedPtr<FJsonObject> Body = MakeShared<FJsonObject>();
    Body->SetStringField(TEXT("message"), Message);
    Body->SetStringField(TEXT("session_id"), SessionId);

    FString BodyString;
    const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&BodyString);
    FJsonSerializer::Serialize(Body.ToSharedRef(), Writer);
    HttpRequest->SetContentAsString(BodyString);

    UE_LOG(LogTemp, Warning, TEXT("POST %s/chat"), *ServerUrl);
    UE_LOG(LogTemp, Warning, TEXT("Request Body: %s"), *BodyString);
    if (!UGoogleCloudAPI::SessionCookie.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("Using Cookie: %s"), *UGoogleCloudAPI::SessionCookie);
    }

    HttpRequest->OnProcessRequestComplete().BindLambda(
        [OnSuccess, OnError, SessionId](FHttpRequestPtr /*Request*/, FHttpResponsePtr Response, bool bWasSuccessful)
        {
            if (!bWasSuccessful || !Response.IsValid())
            {
                OnError.ExecuteIfBound(TEXT("Failed to connect to server"));
                return;
            }

            LogHttpResponse(TEXT("POST /chat"), Response);

            const int32 Code = Response->GetResponseCode();
            const FString Raw = Response->GetContentAsString();
            if (Code < 200 || Code >= 300)
            {
                OnError.ExecuteIfBound(FString::Printf(TEXT("HTTP %d — %s"), Code, *Raw));
                return;
            }

            // Parse JSON
            TSharedPtr<FJsonObject> Json;
            const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Raw);
            if (!FJsonSerializer::Deserialize(Reader, Json) || !Json.IsValid())
            {
                OnError.ExecuteIfBound(TEXT("Failed to parse response JSON"));
                return;
            }

            FChatResponse Out;

            FString ReturnedSid;
            Out.SessionId = (Json->TryGetStringField(TEXT("session_id"), ReturnedSid) && !ReturnedSid.IsEmpty())
                ? ReturnedSid : SessionId;

            Json->TryGetStringField(TEXT("response"), Out.Response);

            const TArray<TSharedPtr<FJsonValue>>* ConvArray = nullptr;
            if (Json->TryGetArrayField(TEXT("conversation"), ConvArray))
            {
                for (const TSharedPtr<FJsonValue>& V : *ConvArray)
                {
                    if (V.IsValid() && V->Type == EJson::String)
                    {
                        Out.Conversation.Add(V->AsString());
                    }
                }
            }

            OnSuccess.ExecuteIfBound(Out);
        });

    HttpRequest->ProcessRequest();
}

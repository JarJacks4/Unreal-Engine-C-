// GoogleCloudAPI.h
#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GoogleCloudAPI.generated.h"

// ----- Response Struct -----
USTRUCT(BlueprintType)
struct FChatResponse
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chat API")
    FString SessionId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chat API")
    FString Response;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chat API")
    TArray<FString> Conversation;
};

// ----- Blueprint Delegates -----
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnChatResponseReceived, const FChatResponse&, ChatResponse);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnSessionCreated, const FString&, SessionId);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnChatError, const FString&, ErrorMessage);

UCLASS()
class ESCAPE_API UGoogleCloudAPI : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    // Base URL for your service
    static const FString ServerUrl;

    // Optional cookie captured from GET /
    static FString SessionCookie;

    /** Creates a new chat session (GET /) and returns session_id */
    UFUNCTION(BlueprintCallable, Category = "Chat API", meta = (DisplayName = "Create Chat Session", AutoCreateRefTerm = "OnSuccess,OnError"))
    static void CreateChatSession(const FOnSessionCreated& OnSuccess,
        const FOnChatError& OnError);

    /** Sends a message (POST /chat) with {message, session_id} and returns response */
    UFUNCTION(BlueprintCallable, Category = "Chat API", meta = (DisplayName = "Send Chat Message", AutoCreateRefTerm = "OnSuccess,OnError"))
    static void SendChatMessage(const FString& Message, const FString& SessionId,
        const FOnChatResponseReceived& OnSuccess,
        const FOnChatError& OnError);
};

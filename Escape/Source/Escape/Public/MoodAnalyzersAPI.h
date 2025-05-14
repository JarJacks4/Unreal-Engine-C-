// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTFunctionLibrary.h"
#include "MoodAnalyzersAPI.generated.h"

/**
 * 
 */
UCLASS()
class ESCAPE_API UMoodAnalyzersAPI : public UBTFunctionLibrary
{
	GENERATED_BODY()
	
};
// GoogleCloudAPI.h
#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Http.h"
#include "Json.h"
#include "JsonUtilities.h"
#include "GoogleCloudAPI.generated.h"

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

// Define the delegate types BEFORE using them in the class definition
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnChatResponseReceived, const FChatResponse&, Response);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnSessionCreated, const FString&, SessionId);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnChatError, const FString&, ErrorMessage);

UCLASS()
class ESCAPE_API UGoogleCloudAPI : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /**
     * Sends a message to the chat API and returns the response
     * @param Message - The message to send to the chat API
     * @param SessionId - The session ID for the conversation (leave empty for new session)
     * @param OnSuccess - Called when the request is successful
     * @param OnError - Called when the request fails
     */
    UFUNCTION(BlueprintCallable, Category = "Chat API", meta = (DisplayName = "Send Chat Message"))
    static void SendChatMessage(const FString& Message, const FString& SessionId,
        const FOnChatResponseReceived& OnSuccess,
        const FOnChatError& OnError);

    /**
     * Creates a new session with the chat API
     * @param OnSuccess - Called when the session is created successfully
     * @param OnError - Called when the session creation fails
     */
    UFUNCTION(BlueprintCallable, Category = "Chat API", meta = (DisplayName = "Create Chat Session"))
    static void CreateChatSession(const FOnSessionCreated& OnSuccess,
        const FOnChatError& OnError);

private:
    static const FString ApiKey;
    static const FString ServerUrl;
};
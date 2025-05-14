// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "JournalingComponent.generated.h"

class AEscapeCharacter;
class USecondCounterComponent;

/**
 * UJournalingComponent
 * Manages the journaling activity, tracking duration and completion.
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ESCAPE_API UJournalingComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UJournalingComponent();

	/** Starts the journaling activity. */
	UFUNCTION(BlueprintCallable, Category = "Journaling")
	void StartJournaling();

	/** Stops the journaling activity. */
	UFUNCTION(BlueprintCallable, Category = "Journaling")
	void StopJournaling();

	/** The duration required for the journaling activity in seconds (10 minutes). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Journaling", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float JournalingDuration = 600.0f;

	/** Points awarded for completing the journaling activity. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring")
	int32 CompletionPoints = 5;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	/** Cached reference to the owning EscapeCharacter. */
	UPROPERTY()
	AEscapeCharacter* CachedEscapeCharacter = nullptr;

	/** Cached reference to the SecondCounterComponent. */
	UPROPERTY()
	USecondCounterComponent* SecondCounterComponent = nullptr;
};

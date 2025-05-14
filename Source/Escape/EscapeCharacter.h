// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "WellnessBlock.h"
#include "Widgets/MobileUIWidget.h"
#include "Widgets/ActivityUIWidget.h"
#include "Widgets/ScoreWidget.h"
#include "Components/SecondCounterComponent.h"
#include "EscapeCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class UMeditationComponent;
class UStretchingComponent;
class UDeepBreathingComponent;
class UWellnessComponent;
class UJournalingComponent;
class UInteractionWidget;
class UMessageWidget;

/**
 * Enum representing the possible states of the character related to wellness activities.
 */
UENUM(BlueprintType)
enum class EMinuteGoalActionsState : uint8
{
	Idle UMETA(DisplayName = "Idle"),
	Meditating UMETA(DisplayName = "Meditating"),
	DeepBreathing UMETA(DisplayName = "Deep Breathing"),
	Stretching UMETA(DisplayName = "Stretching"),
	Journaling UMETA(DisplayName = "Journaling")
};

/**
 * The main player character class for the Escape game.
 */
UCLASS(config = Game)
class ESCAPE_API AEscapeCharacter : public ACharacter
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> InputMapping;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> LeftAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> RightAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> UpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> DownAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MinuteGoalActions;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MeditationTilt;

public:
	AEscapeCharacter();

	virtual void Tick(float DeltaTime) override;

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Wellness")
	void Activity();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Character State")
	float GetPlayerDeltaTime() const { return DeltaTimePlayer; }

	UFUNCTION(BlueprintCallable, Category = "Character State")
	void SetPlayerDeltaTime(float delta) { DeltaTimePlayer = delta; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Character State|Input")
	float GetTilt() const { return TiltInput; }

	UFUNCTION(BlueprintCallable, Category = "Character State|Input")
	void SetTilt(float Tilt) { TiltInput = Tilt; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Character State|Wellness")
	EMinuteGoalActionsState GetMinuteGoalActionsState() const { return MinuteGoalActionsState; }

	UFUNCTION(BlueprintCallable, Category = "Character State|Wellness")
	void SetMinuteGoalActionState(EMinuteGoalActionsState NMinuteGoalActionsState) { MinuteGoalActionsState = NMinuteGoalActionsState; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Character State|Wellness")
	EWellnessBlockType GetBlockType() const { return BlockType; }

	UFUNCTION(BlueprintCallable, Category = "Character State|Wellness")
	void SetBlockType(EWellnessBlockType NewBlockType) { BlockType = NewBlockType; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "UI")
	UActivityUIWidget* GetActivityUIWidget() const { return ActivityUIWidget.Get(); }

	UFUNCTION(BlueprintCallable, Category = "UI")
	void SetActivityUIWidget(UActivityUIWidget* ActivityP) { ActivityUIWidget = ActivityP; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "UI")
	UMobileUIWidget* GetMobileUIWidget() const { return MobileUIWidget.Get(); }

	UFUNCTION(BlueprintCallable, Category = "UI")
	void SetMobileUIWidget(UMobileUIWidget* MobileP) { MobileUIWidget = MobileP; }

	UPROPERTY(BlueprintReadOnly, Category = "Wellness|Score")
	float AggregatedScore = 0.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Character State|Interaction")
	bool bIsInBlock = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components|Wellness", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMeditationComponent> MeditationComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components|Wellness", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStretchingComponent> StretchingComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components|Wellness", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UDeepBreathingComponent> DeepBreathingComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components|Wellness", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UWellnessComponent> WellnessComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components|Wellness", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UJournalingComponent> JournalingComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components|Timers", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USecondCounterComponent> SecondCounterComponent;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UMobileUIWidget> MobileUIWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UActivityUIWidget> ActivityUIWidgetClass;

protected:
	virtual void Jump() override;
	virtual void StopJumping() override;

	void Move(const FInputActionValue& Value);

	void Look(const FInputActionValue& Value);

	void HandleLeftInput(const FInputActionValue& Value);
	void HandleRightInput(const FInputActionValue& Value);
	void HandleUpInput(const FInputActionValue& Value);
	void HandleDownInput(const FInputActionValue& Value);
	void HandleTiltInput(const FInputActionValue& Value);

	virtual void NotifyControllerChanged() override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	EMinuteGoalActionsState MinuteGoalActionsState = EMinuteGoalActionsState::Idle;
	float DeltaTimePlayer = 0.0f;
	float TiltInput = 0.0f;
	EWellnessBlockType BlockType = EWellnessBlockType::None;

	UPROPERTY(Transient)
	TObjectPtr<UMobileUIWidget> MobileUIWidget;

	UPROPERTY(Transient)
	TObjectPtr<UActivityUIWidget> ActivityUIWidget;

public:
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom.Get(); }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera.Get(); }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool OnScreenDebugBool = true;
};

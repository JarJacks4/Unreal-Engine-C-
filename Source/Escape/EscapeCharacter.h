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
class UInteractionWidget;
class UMessageWidget;

/**
 * Enum representing the possible states of the meditation mechanic.
 */
UENUM(BlueprintType)
enum class EMinuteGoalActionsState : uint8
{
	Idle        UMETA(DisplayName = "Idle"),        // Not realizing NY minute actions
	Meditating  UMETA(DisplayName = "Meditating"),  // Actively meditating
	DeepBreathing  UMETA(DisplayName = "Deep Breathing"),  // Actively deepbreathing
	Stretching  UMETA(DisplayName = "Stretching")   // Actively stretching
};


UCLASS(config=Game)
class AEscapeCharacter : public ACharacter
{
	GENERATED_BODY()


	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

public:
	AEscapeCharacter();

	/**
	 * Called every frame to update character logic, including wellness activities.
	 */
	virtual void Tick(float DeltaTime) override;
	/**
	 * Called when the character is created
	*/
	virtual void BeginPlay() override;

	/**
	 * Starts the specified wellness activity (meditation,deep breathing or stretching).
	 */
	UFUNCTION(BlueprintCallable)
	void Activity();

	UFUNCTION(BlueprintCallable, Category = "Minute Goal Actions")
	EMinuteGoalActionsState GetMinuteGoalActionsState() { return MinuteGoalActionsState; }

	UFUNCTION(BlueprintCallable, Category = "Minute Goal Actions")
	EMinuteGoalActionsState SetMinuteGoalActionState(EMinuteGoalActionsState NMinuteGoalActionsState ) { return MinuteGoalActionsState = NMinuteGoalActionsState; }

	UFUNCTION(BlueprintCallable, Category = "Wellness Block")
	EWellnessBlockType SetWellnessBlockType(EWellnessBlockType BlockType) { return BlockTypePlayer = BlockType; }

	UFUNCTION(BlueprintCallable, Category = "Wellness Block")
	EWellnessBlockType GetBlockType() { return BlockTypePlayer; }

	UFUNCTION(BlueprintCallable)
	UActivityUIWidget* GetActivityUIWidget() { return ActivityUIWidget; }

	UFUNCTION(BlueprintCallable)
	void SetActivityUIWidget(UActivityUIWidget* ActivityP) { ActivityUIWidget = ActivityP; }

	UFUNCTION(BlueprintCallable)
	UMobileUIWidget* GetMobileUIWidget() { return MobileUIWidget; }

	UFUNCTION(BlueprintCallable)
	void SetMobileUIWidget(UMobileUIWidget* MobileP) { MobileUIWidget =  MobileP; }


	/**
	 * Meditation component responsible for handling the meditation mechanic.
	 * Exposed to Blueprints for easy access and configuration.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UMeditationComponent* MeditationComponent;

	/**
	 * Input action specifically for triggering the activity mechanics.
	 * Bound to start/stop activity functions via Enhanced Input.
	 * Assigned in the Editor or Blueprint.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<class UInputAction> MinuteGoalActions;

	/**
	* Stretching component responsible for handling the Stretching mechanic.
	* Exposed to Blueprints for easy access and configuration.
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr <class UStretchingComponent> StretchingComponent;

	/**
	* Stretching component responsible for handling the Meditation Timer.
	* Exposed to Blueprints for easy access and configuration.
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr <class USecondCounterComponent> SecondCounterMeditation;
	
	/**
	* Stretching component responsible for handling the Stretching timer.
	* Exposed to Blueprints for easy access and configuration.
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr <class USecondCounterComponent> SecondCounterStretching;

	/**
	* Stretching component responsible for handling the Deep Breathing timer.
	* Exposed to Blueprints for easy access and configuration.
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr <class USecondCounterComponent> SecondCounterBreathing;

	/**
	* Deep Breathing component responsible for handling the deep breathing mechanic.
	* Exposed to Blueprints for easy access and configuration.
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UDeepBreathingComponent> DeepBreathingComponent;

	/**
	* Deep Breathing component responsible for handling the wellness messages.
	* Exposed to Blueprints for easy access and configuration.
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UWellnessComponent> WellnessComponent;

	bool bIsInBlock = false;

	// Class of the affirmation widget to create
	// Must be assigned in the Editor's details panel
	UPROPERTY(EditAnywhere)
	TSubclassOf<UMobileUIWidget> MobileUIWidgetClass;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UActivityUIWidget> ActivityUIWidgetClass;

private:
	EWellnessBlockType BlockTypePlayer;
	EMinuteGoalActionsState MinuteGoalActionsState;
	TArray<AActor*> WellnessBlc;



	// Currently displayed affirmation widget
	UMobileUIWidget* MobileUIWidget;


	// Currently displayed affirmation widget
	UActivityUIWidget* ActivityUIWidget;


protected:
	//**Function called to start jumping */
	void Jump();
	//** Function called to stop jumping */
	void StopJumping();
	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);
			

protected:

	virtual void NotifyControllerChanged() override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};


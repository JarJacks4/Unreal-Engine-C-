// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "WellnessBlock.h" // Includes EWellnessBlockType
#include "Widgets/MobileUIWidget.h" // Forward declarations are good, but include needed for TSubclassOf
#include "Widgets/ActivityUIWidget.h" // Forward declarations are good, but include needed for TSubclassOf
#include "Widgets/ScoreWidget.h" // Forward declaration
#include "Components/SecondCounterComponent.h" // Forward declaration
#include "EscapeCharacter.generated.h"

// Forward declarations for components and input assets
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class UMeditationComponent;
class UStretchingComponent;
class UDeepBreathingComponent;
class UWellnessComponent;
class UJournalingComponent; // Forward declare JournalingComponent
class UInteractionWidget; // Already in MobileUIWidget.h, but good practice
class UMessageWidget; // Forward declaration

/**
 *  Enum representing the possible states of the character related to wellness activities.
 * Used to track whether the character is idle or engaged in a specific activity like meditating, deep breathing, or stretching.
 */
UENUM(BlueprintType)
enum class EMinuteGoalActionsState : uint8
{
	/**  The character is not currently engaged in any wellness activity. */
	Idle        UMETA(DisplayName = "Idle"),
	/**  The character is actively performing the meditation activity. */
	Meditating  UMETA(DisplayName = "Meditating"),
	/**  The character is actively performing the deep breathing activity. */
	DeepBreathing  UMETA(DisplayName = "Deep Breathing"),
	/**  The character is actively performing the stretching activity. */
	Stretching  UMETA(DisplayName = "Stretching"),
	/**  The character is actively performing the journaling activity. */
	Journaling  UMETA(DisplayName = "Journaling")
};


/**
 *  The main player character class for the Escape game.
 * Handles player movement, input, camera control, and interaction with wellness mechanics (meditation, stretching, deep breathing).
 * Integrates various components (Meditation, Stretching, DeepBreathing, Wellness, SecondCounter) and UI widgets (Mobile, Activity).
 */
UCLASS(config = Game) // Specifies that this class can have properties saved in configuration files (e.g., DefaultGame.ini)
class ESCAPE_API AEscapeCharacter : public ACharacter
{
	GENERATED_BODY()

	// --- Camera Components ---
	/**  Spring arm component that positions the camera behind the character, providing smooth camera movement and collision handling. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;

	/**  The main camera component attached to the CameraBoom, providing the player's view. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FollowCamera;

	// --- Input Actions & Contexts ---
	/**  The default Input Mapping Context used for basic character controls like movement and looking. Assign in Blueprint Editor. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	/**  Input Action asset for the jump action. Assign in Blueprint Editor. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> JumpAction;

	/**  Input Action asset for the movement action (forward/backward, left/right). Assign in Blueprint Editor. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MoveAction;

	/**  Input Action asset for the camera look action (mouse/gamepad look). Assign in Blueprint Editor. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> LookAction;

	/**  Input Mapping Context specifically for wellness activity inputs (e.g., stretching directions, activity toggle). Assign in Blueprint Editor. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> InputMapping;

	/**  Input Action for the 'stretch left' command. Assign in Blueprint Editor. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> LeftAction;

	/**  Input Action for the 'stretch right' command. Assign in Blueprint Editor. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> RightAction;

	/**  Input Action for the 'stretch up' command. Assign in Blueprint Editor. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> UpAction;

	/**  Input Action for the 'stretch down' command. Assign in Blueprint Editor. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> DownAction;

	/**  Input Action asset used to trigger the main 'Activity' function, starting/stopping wellness activities. Assign in Blueprint Editor. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MinuteGoalActions;

	/**  Input Action asset for handling tilt input during meditation. Assign in Blueprint Editor. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MeditationTilt;



	// --- Public Methods & Properties ---
public:
	/**  Constructor for the AEscapeCharacter class. Initializes components and default values. */
	AEscapeCharacter();

	/**
	 *  Called every frame. Updates character logic, handles input, and manages ongoing wellness activities.
	 *  DeltaTime Game time elapsed during the last frame.
	 */
	virtual void Tick(float DeltaTime) override;

	/**
	 *  Called when the game starts or when spawned. Used for initialization tasks like setting up components, UI, and input.
	 */
	virtual void BeginPlay() override;

	/**
	 *  Initiates or toggles the current wellness activity based on the character's state and the interacting WellnessBlock type.
	 * This function acts as the primary trigger for starting/stopping meditation, deep breathing, or stretching.
	 */
	UFUNCTION(BlueprintCallable, Category = "Wellness")
	void Activity();

	// --- Accessors & Mutators for Character State ---

	/**  Gets the cached delta time for the last frame. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Character State") // Added BlueprintPure
	float GetPlayerDeltaTime() const { return DeltaTimePlayer; }

	/**  Sets the cached delta time. Typically called internally within Tick. */
	UFUNCTION(BlueprintCallable, Category = "Character State")
	void SetPlayerDeltaTime(float delta) { DeltaTimePlayer = delta; }

	/**  Gets the current tilt input value, used for mechanics like meditation balance. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Character State|Input") // Added BlueprintPure
	float GetTilt() const { return TiltInput; }

	/**  Sets the tilt input value. Typically called by input handling functions. */
	UFUNCTION(BlueprintCallable, Category = "Character State|Input")
	void SetTilt(float Tilt) { TiltInput = Tilt; }

	/**  Gets the current wellness activity state of the character (Idle, Meditating, etc.). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Character State|Wellness") // Added BlueprintPure
	EMinuteGoalActionsState GetMinuteGoalActionsState() const { return MinuteGoalActionsState; }

	/**  Sets the current wellness activity state of the character. Should be called by activity components when starting/stopping. */
	UFUNCTION(BlueprintCallable, Category = "Character State|Wellness")
	void SetMinuteGoalActionState(EMinuteGoalActionsState NMinuteGoalActionsState) { MinuteGoalActionsState = NMinuteGoalActionsState; }

	/**  Gets the current wellness block type the character is interacting with. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Character State|Wellness")
	EWellnessBlockType GetBlockType() const { return BlockType; }

	/**  Sets the current wellness block type. Typically called when interacting with a WellnessBlock. */
	UFUNCTION(BlueprintCallable, Category = "Character State|Wellness")
	void SetBlockType(EWellnessBlockType NewBlockType) { BlockType = NewBlockType; }

	// --- Accessors & Mutators for UI Widgets ---

	/**  Gets a pointer to the currently active Activity UI widget instance. Returns null if not created. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "UI") // Added BlueprintPure
	UActivityUIWidget* GetActivityUIWidget() const { return ActivityUIWidget.Get(); } // Use .Get() for TObjectPtr

	/**  Sets the pointer to the active Activity UI widget instance. Typically called internally during BeginPlay. */
	UFUNCTION(BlueprintCallable, Category = "UI") // Consider making protected if only used internally
	void SetActivityUIWidget(UActivityUIWidget* ActivityP) { ActivityUIWidget = ActivityP; }

	/**  Gets a pointer to the currently active Mobile UI widget instance. Returns null if not created. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "UI") // Added BlueprintPure
	UMobileUIWidget* GetMobileUIWidget() const { return MobileUIWidget.Get(); } // Use .Get() for TObjectPtr

	/**  Sets the pointer to the active Mobile UI widget instance. Typically called internally during BeginPlay. */
	UFUNCTION(BlueprintCallable, Category = "UI") // Consider making protected if only used internally
	void SetMobileUIWidget(UMobileUIWidget* MobileP) { MobileUIWidget = MobileP; }

	/** Aggregated score for all completed activities */
	UPROPERTY(BlueprintReadOnly, Category = "Wellness|Score")
	float AggregatedScore = 0.0f;

	// --- Public Properties ---

	/**  Flag indicating if the character is currently inside the trigger volume of a WellnessBlock. Updated by WellnessBlock overlap events. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Character State|Interaction") // Expose to BP for debugging/logic
		bool bIsInBlock = false;

	// --- Wellness Components ---
	/**
	 *  Component responsible for managing the logic and state of the meditation activity.
	 * Handles input (tilt), timing, music, and UI updates for meditation.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components|Wellness", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMeditationComponent> MeditationComponent;

	/**
	*  Component responsible for managing the logic and state of the stretching activity (rhythm game).
	* Handles directional input, interacts with RythmWidget, manages music, and scoring.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components|Wellness", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStretchingComponent> StretchingComponent;

	/**
	*  Component responsible for managing the logic and state of the deep breathing activity.
	* Handles the inhale/exhale cycle, bubble scaling, tapping interaction, music, and scoring.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components|Wellness", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UDeepBreathingComponent> DeepBreathingComponent;

	/**
	*  Component responsible for managing and displaying periodic wellness messages (hydration, gratitude, affirmations) via the MessageWidget.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components|Wellness", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UWellnessComponent> WellnessComponent;

	/**
	*  Component responsible for managing the logic and state of the journaling activity.
	* Handles UI display, text input/saving, and scoring.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components|Wellness", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UJournalingComponent> JournalingComponent;

	// --- Timer Components ---
	/**
	*  Timer component specifically used to track and score the duration of the meditation activity.
	* Integrates with the ActivityUIWidget's ScoreWidget. Configured with SaveSlotName "MeditationScore".
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components|Timers", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USecondCounterComponent> SecondCounterComponent;

	// --- UI Widget Classes & Instances ---
	/**
	 *  The UUserWidget class to instantiate for the mobile phone UI (interaction prompts).
	 * Must be assigned in the Blueprint Editor.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "UI") // EditDefaultsOnly as it's a class reference
		TSubclassOf<UMobileUIWidget> MobileUIWidgetClass;

	/**
	 *  The UUserWidget class to instantiate for the activity-specific UI (e.g., meditation balance, breathing bubble, score display).
	 * Must be assigned in the Blueprint Editor.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "UI") // EditDefaultsOnly as it's a class reference
		TSubclassOf<UActivityUIWidget> ActivityUIWidgetClass;

	// --- Protected Input Handlers ---
protected:
	/**  Overrides the default ACharacter::Jump to add state checks (only jump if Idle). */
	virtual void Jump() override;
	/**  Overrides the default ACharacter::StopJumping. */
	virtual void StopJumping() override;
	/**
	 *  Handles movement input (forward/backward, left/right). Only allows movement if Idle.
	 *  Value The input value containing the movement vector.
	 */
	void Move(const FInputActionValue& Value);

	/**
	 *  Handles camera look input (pitch/yaw).
	 *  Value The input value containing the look vector.
	 */
	void Look(const FInputActionValue& Value);

	/**  Handles the input action for stretching left. Sets state on StretchingComponent. */
	void HandleLeftInput(const FInputActionValue& Value);
	/**  Handles the input action for stretching right. Sets state on StretchingComponent. */
	void HandleRightInput(const FInputActionValue& Value);
	/**  Handles the input action for stretching up. Sets state on StretchingComponent. */
	void HandleUpInput(const FInputActionValue& Value);
	/**  Handles the input action for stretching down. Sets state on StretchingComponent. */
	void HandleDownInput(const FInputActionValue& Value);
	/**  Handles the input action for tilt, used during meditation. Updates TiltInput variable. */
	void HandleTiltInput(const FInputActionValue& Value);

	// --- Protected Engine Overrides ---

	/**  Called when the Controller controlling this Pawn changes. Used to update input mappings. */
	virtual void NotifyControllerChanged() override;

	/**
	 *  Called to bind functionality to input events. Sets up bindings for movement, looking, jumping, and wellness activities using Enhanced Input.
	 *  PlayerInputComponent The input component to bind actions to.
	 */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// --- Private Internal State ---
private:
	/**  Stores the current wellness activity state of the character (Idle, Meditating, etc.). */
	EMinuteGoalActionsState MinuteGoalActionsState = EMinuteGoalActionsState::Idle; // Initialize to Idle
	/**  Cached delta time from the last Tick. */
	float DeltaTimePlayer = 0.0f;
	/**  Stores the current tilt input value, processed from input actions. */
	float TiltInput = 0.0f;
	/**  Stores the current wellness block type the character is interacting with. */
	EWellnessBlockType BlockType = EWellnessBlockType::None; // Initialize to None

	/**  Pointer to the currently instantiated Mobile UI widget. Null if not created or destroyed. */
	UPROPERTY(Transient) // Mark as Transient as it's created dynamically and shouldn't be saved/serialized.
		TObjectPtr<UMobileUIWidget> MobileUIWidget;

	/**  Pointer to the currently instantiated Activity UI widget. Null if not created or destroyed. */
	UPROPERTY(Transient) // Mark as Transient.
		TObjectPtr<UActivityUIWidget> ActivityUIWidget;

	// --- Public Accessors for Components (ForceInline for potential performance) ---
public:
	/**  Returns the CameraBoom subobject. */
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom.Get(); } // Use .Get() with TObjectPtr
	/**  Returns the FollowCamera subobject. */
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera.Get(); } // Use .Get() with TObjectPtr

	/** Enables or disables on-screen debug messages for the entire game. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool OnScreenDebugBool = true;
};

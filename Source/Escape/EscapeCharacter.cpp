// Copyright Epic Games, Inc. All Rights Reserved.

#include "EscapeCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

#include "Components/MeditationComponent.h"
#include "Components/StretchingComponent.h"
#include "Components/DeepBreathingComponent.h"
#include "Components/WellnessComponent.h"
#include "Components/SecondCounterComponent.h"
#include "Components/JournalingComponent.h" // Include JournalingComponent header

#include "Widgets/MobileUIWidget.h"
#include "Widgets/ActivityUIWidget.h"
#include "Widgets/ScoreWidget.h"
#include "Widgets/BreathingBubbleWidget.h"
#include "Widgets/InteractionWidget.h"
// #include "Widgets/JournalingWidget.h" // TODO: Include when JournalingWidget exists

#include "WellnessBlock.h"
#include "WellnessSaveGame.h"

/**
 *  Constructor for the AEscapeCharacter class.
 * Initializes default values, sets up collision capsule, configures character movement,
 * creates camera components (boom and follow camera), and instantiates wellness-related components
 * (Meditation, DeepBreathing, Stretching, Wellness, SecondCounter timers, Journaling). Also configures save slots for timers.
 */
AEscapeCharacter::AEscapeCharacter()
{
	// Initialize the collision capsule size.
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Configure character rotation settings. Prevent the character mesh from rotating with the controller.
	// Rotation is typically handled by the camera boom and movement component.
	bUseControllerRotationPitch = false; // Camera rotation controls pitch.
	bUseControllerRotationYaw = false;   // Movement component or explicit code controls yaw.
	bUseControllerRotationRoll = false;  // Camera rotation controls roll.

	// Configure character movement component settings.
	GetCharacterMovement()->bOrientRotationToMovement = true; // Make the character mesh face the direction of movement.
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // Set the rotation speed when orienting to movement.

	// Set default movement parameters. These can be adjusted in the derived Blueprint asset.
	GetCharacterMovement()->JumpZVelocity = 700.f; // Initial vertical velocity on jump.
	GetCharacterMovement()->AirControl = 0.35f;    // Amount of movement control while airborne.
	GetCharacterMovement()->MaxWalkSpeed = 500.f;   // Maximum ground speed.
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f; // Minimum speed when using analog input.
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f; // How quickly the character stops when walking.
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f; // How quickly the character stops falling (less relevant without friction).

	// Create the camera boom component.
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent); // Attach to the root (capsule).
	CameraBoom->TargetArmLength = 400.0f;       // Set the default distance from the character.
	CameraBoom->bUsePawnControlRotation = true; // Allow the controller's rotation (pitch/yaw) to control the boom's rotation.

	// Create the follow camera component.
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false; // Prevent the camera itself from rotating based on pawn control rotation; the boom handles it.

	// Create instances of the wellness-related components.
	MeditationComponent = CreateDefaultSubobject<UMeditationComponent>(TEXT("MeditationComponent"));
	DeepBreathingComponent = CreateDefaultSubobject<UDeepBreathingComponent>(TEXT("DeepBreathingComponent"));
	StretchingComponent = CreateDefaultSubobject<UStretchingComponent>(TEXT("StretchingComponent"));
	WellnessComponent = CreateDefaultSubobject<UWellnessComponent>(TEXT("WellnessComponent"));
	SecondCounterComponent = CreateDefaultSubobject<USecondCounterComponent>(TEXT("SecondCounterComponent"));
	JournalingComponent = CreateDefaultSubobject<UJournalingComponent>(TEXT("JournalingComponent")); // Create JournalingComponent instance

	// Initialize state
	MinuteGoalActionsState = EMinuteGoalActionsState::Idle;
	bIsInBlock = false;
	TiltInput = 0.0f;
	DeltaTimePlayer = 0.0f;
}

/**
 *  Called every frame. Updates the base character tick and caches the delta time.
 *  DeltaTime The time elapsed since the last frame.
 */
void AEscapeCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	SetPlayerDeltaTime(DeltaTime);
}

/**
 *  Called when the game starts or when the character is spawned.
 * Handles initialization tasks such as setting up input subsystems and creating UI widgets.
 */
void AEscapeCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Load AggregatedScore from save
	const FString WellnessSaveSlot = TEXT("WellnessSaveData");
	const int32 WellnessUserIndex = 0;
	UWellnessSaveGame* SaveGameInstance = Cast<UWellnessSaveGame>(UGameplayStatics::LoadGameFromSlot(WellnessSaveSlot, WellnessUserIndex));
	if (SaveGameInstance)
	{
		AggregatedScore = SaveGameInstance->AggregatedScore;
	}
	else
	{
		AggregatedScore = 0.0f;
	}

	// Platform-specific UI setup for mobile devices.
	if (UGameplayStatics::GetPlatformName() == "Android" || UGameplayStatics::GetPlatformName() == "IOS" || UGameplayStatics::GetPlatformName() == "Windows" )
	{
		if (MobileUIWidgetClass)
		{
			MobileUIWidget = CreateWidget<UMobileUIWidget>(GetWorld(), MobileUIWidgetClass);
			if (MobileUIWidget)
			{
				MobileUIWidget->AddToViewport(1);
				MobileUIWidget->SetPlayer(this);
				if (MobileUIWidget->InteractionWidget)
				{
					MobileUIWidget->InteractionWidget->SetPlayer(this);
				}
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("AEscapeCharacter::BeginPlay - Failed to create MobileUIWidget instance."));
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("AEscapeCharacter::BeginPlay - MobileUIWidgetClass is not set. Mobile UI will not be created."));
		}
	}

	// Create and configure the Activity UI widget (used for meditation, breathing, stretching, journaling visuals).
	if (ActivityUIWidgetClass)
	{
		ActivityUIWidget = CreateWidget<UActivityUIWidget>(GetWorld(), ActivityUIWidgetClass);
		if (ActivityUIWidget)
		{
			ActivityUIWidget->AddToViewport();
			ActivityUIWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible); // Always visible.

			// Hide bubble button, bubble target, and bubble tap at game start
			if (UBreathingBubbleWidget* BreathingWidget = ActivityUIWidget->GetDeepBreathingWidget())
			{
				if (BreathingWidget->BubbleButton)
					BreathingWidget->BubbleButton->SetVisibility(ESlateVisibility::Collapsed);
			}
			if (UButton* TapButton = ActivityUIWidget->GetTapButton())
			{
				TapButton->SetVisibility(ESlateVisibility::Collapsed);
			}
			if (UImage* TargetBubble = ActivityUIWidget->GetTargetBubble())
			{
				TargetBubble->SetVisibility(ESlateVisibility::Collapsed);
			}

			// Link the timer components to the score display widget within the Activity UI.
			if (UScoreWidget* ScoreWidget = ActivityUIWidget->GetScoreWidget())
			{
				ScoreWidget->UpdateScore(AggregatedScore);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("AEscapeCharacter::BeginPlay - ActivityUIWidget is missing its ScoreWidget child. Timers cannot display score."));
			}

			// Set the player reference in the nested Deep Breathing widget.
			if (UBreathingBubbleWidget* BreathingWidget = ActivityUIWidget->GetDeepBreathingWidget())
			{
				BreathingWidget->SetPlayer(this);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("AEscapeCharacter::BeginPlay - ActivityUIWidget is missing its BreathingBubbleWidget child."));
			}

			// TODO: Add Journaling Widget setup here when it exists
			// Example:
			// if (UJournalingWidget* JournalingWidget = ActivityUIWidget->GetJournalingWidget())
			// {
			// 	JournalingComponent->SetJournalingWidget(JournalingWidget);
			// 	JournalingWidget->SetVisibility(ESlateVisibility::Collapsed);
			// }
			// else
			// {
			// 	UE_LOG(LogTemp, Warning, TEXT("AEscapeCharacter::BeginPlay - ActivityUIWidget is missing its JournalingWidget child."));
			// }
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("AEscapeCharacter::BeginPlay - Failed to create ActivityUIWidget instance."));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AEscapeCharacter::BeginPlay - ActivityUIWidgetClass is not set. Activity UI will not be created."));
	}

	// Add input mapping context after ensuring the controller is valid.
	NotifyControllerChanged();
}

/**
 *  Binds input actions to their corresponding handler functions using the Enhanced Input system.
 * This function is called by the engine to set up player input.
 *  PlayerInputComponent The component responsible for handling player input (expected to be UEnhancedInputComponent).
 */
void AEscapeCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Attempt to cast the input component to the Enhanced Input Component type.
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Bind the main 'Activity' action (used to start/stop wellness activities).
		if (MinuteGoalActions)
		{
			EnhancedInputComponent->BindAction(MinuteGoalActions, ETriggerEvent::Started, this, &AEscapeCharacter::Activity);
		}
		else { UE_LOG(LogTemp, Warning, TEXT("Input Action 'MinuteGoalActions' is not assigned.")); }

		// Bind the 'Meditation Tilt' action.
		if (MeditationTilt)
		{
			EnhancedInputComponent->BindAction(MeditationTilt, ETriggerEvent::Triggered, this, &AEscapeCharacter::HandleTiltInput);
		}
		else { UE_LOG(LogTemp, Warning, TEXT("Input Action 'MeditationTilt' is not assigned.")); }

		// Bind stretching direction actions. Using Triggered for continuous hold.
		if (LeftAction) EnhancedInputComponent->BindAction(LeftAction, ETriggerEvent::Triggered, this, &AEscapeCharacter::HandleLeftInput);
		else { UE_LOG(LogTemp, Warning, TEXT("Input Action 'LeftAction' is not assigned.")); }

		if (RightAction) EnhancedInputComponent->BindAction(RightAction, ETriggerEvent::Triggered, this, &AEscapeCharacter::HandleRightInput);
		else { UE_LOG(LogTemp, Warning, TEXT("Input Action 'RightAction' is not assigned.")); }

		if (UpAction) EnhancedInputComponent->BindAction(UpAction, ETriggerEvent::Triggered, this, &AEscapeCharacter::HandleUpInput);
		else { UE_LOG(LogTemp, Warning, TEXT("Input Action 'UpAction' is not assigned.")); }

		if (DownAction) EnhancedInputComponent->BindAction(DownAction, ETriggerEvent::Triggered, this, &AEscapeCharacter::HandleDownInput);
		else { UE_LOG(LogTemp, Warning, TEXT("Input Action 'DownAction' is not assigned.")); }

		// Bind Jumping.
		if (JumpAction)
		{
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AEscapeCharacter::Jump);
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AEscapeCharacter::StopJumping);
		}
		else { UE_LOG(LogTemp, Warning, TEXT("Input Action 'JumpAction' is not assigned.")); }

		// Bind Moving.
		if (MoveAction)
		{
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AEscapeCharacter::Move);
		}
		else { UE_LOG(LogTemp, Warning, TEXT("Input Action 'MoveAction' is not assigned.")); }

		// Bind Looking.
		if (LookAction)
		{
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AEscapeCharacter::Look);
		}
		else { UE_LOG(LogTemp, Warning, TEXT("Input Action 'LookAction' is not assigned.")); }
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("'%s' Failed to find an Enhanced Input component! Input bindings will not work."), *GetNameSafe(this));
	}
}

/**
 *  Called when the controller possessing this character changes.
 * Ensures the necessary Input Mapping Contexts are added to the Enhanced Input Subsystem.
 */
void AEscapeCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	// Get the Player Controller associated with this character.
	APlayerController* PlayerController = Cast<APlayerController>(Controller);
	if (!PlayerController)
	{
		return;
	}

	// Get the Enhanced Input Local Player Subsystem.
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
	if (!Subsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("NotifyControllerChanged: Failed to get Enhanced Input Subsystem for %s."), *GetNameSafe(this));
		return;
	}

	// Clear existing mappings to prevent duplicates if the controller changes multiple times or on restarts.
	Subsystem->ClearAllMappings();

	// Add the DefaultMappingContext (for movement, looking, jumping) with priority 0.
	if (DefaultMappingContext)
	{
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("NotifyControllerChanged: DefaultMappingContext is not assigned in %s."), *GetNameSafe(this));
	}

	// Add the specific InputMapping context (for wellness activities) with priority 1.
	if (InputMapping)
	{
		Subsystem->AddMappingContext(InputMapping, 1);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("NotifyControllerChanged: InputMapping context for wellness activities is not assigned in %s."), *GetNameSafe(this));
	}
}

/**
 *  Handles the input event for the 'Left' action, typically used for stretching left.
 * Sets the stretch state on the StretchingComponent if the input is active.
 *  Value The input action value, expected to be a boolean.
 */
void AEscapeCharacter::HandleLeftInput(const FInputActionValue& Value)
{
	// Check if the input is active (e.g., key pressed/held).
	if (Value.Get<bool>() && StretchingComponent)
	{
		// If active, tell the StretchingComponent to enter the StretchLeft state.
		StretchingComponent->SetStretchState(EStretchState::StretchLeft);
	}
}

/**
 *  Handles the input event for the 'Right' action, typically used for stretching right.
 * Sets the stretch state on the StretchingComponent if the input is active.
 *  Value The input action value, expected to be a boolean.
 */
void AEscapeCharacter::HandleRightInput(const FInputActionValue& Value)
{
	if (Value.Get<bool>() && StretchingComponent)
	{
		StretchingComponent->SetStretchState(EStretchState::StretchRight);
	}
}

/**
 *  Handles the input event for the 'Up' action, typically used for stretching up.
 * Sets the stretch state on the StretchingComponent if the input is active.
 *  Value The input action value, expected to be a boolean.
 */
void AEscapeCharacter::HandleUpInput(const FInputActionValue& Value)
{
	if (Value.Get<bool>() && StretchingComponent)
	{
		StretchingComponent->SetStretchState(EStretchState::StretchUp);
	}
}

/**
 *  Handles the input event for the 'Down' action, typically used for stretching down.
 * Sets the stretch state on the StretchingComponent if the input is active.
 *  Value The input action value, expected to be a boolean.
 */
void AEscapeCharacter::HandleDownInput(const FInputActionValue& Value)
{
	if (Value.Get<bool>() && StretchingComponent)
	{
		StretchingComponent->SetStretchState(EStretchState::StretchDown);
	}
}

/**
 *  Handles the input event for 'Tilt', used during meditation. Updates the internal TiltInput variable.
 *  Value The input action value, expected to be a float representing the tilt amount (e.g., from device sensors or mouse movement).
 */
void AEscapeCharacter::HandleTiltInput(const FInputActionValue& Value)
{
	// Update the character's internal TiltInput variable with the value from the input action.
	// The WellnessBlock reads this value in its Tick to control rotation during meditation.
	SetTilt(Value.Get<float>());
}

/**
 *  Toggles the current wellness activity based on the character's state and context (interacting block type).
 * If idle and interacting with a block (or no block type specified, defaulting to Deep Breathing), starts the corresponding activity.
 * If already performing an activity, stops it.
 */
void AEscapeCharacter::Activity()
{
	// Validate Components
	// Ensure essential components are valid before proceeding.
	if (!MeditationComponent || !StretchingComponent || !DeepBreathingComponent || !JournalingComponent) // Added JournalingComponent check
	{
		UE_LOG(LogTemp, Error, TEXT("AEscapeCharacter::Activity() called but one or more core wellness components are invalid!"));
		return;
	}
	if (!GetCharacterMovement())
	{
		UE_LOG(LogTemp, Error, TEXT("AEscapeCharacter::Activity() called but CharacterMovementComponent is invalid!"));
		return;
	}

	// Start Activity Logic
	// Check if the character is currently idle and not falling (can't start activities mid-air).
	if (MinuteGoalActionsState == EMinuteGoalActionsState::Idle && !GetCharacterMovement()->IsFalling())
	{
		// Determine which activity to start based on the currently interacting WellnessBlock type.
		switch (BlockType)
		{
		case EWellnessBlockType::Meditation:
			MeditationComponent->StartMeditation();
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Meditation started"));
			return;
		case EWellnessBlockType::Stretching:
			StretchingComponent->StartStretching();
			return;
		case EWellnessBlockType::Journaling: 
			JournalingComponent->StartJournaling(); 
			return;
		case EWellnessBlockType::None: 
			// Default to Deep Breathing if no specific block type is set.
			// This allows the player to start deep breathing without a specific block.
			DeepBreathingComponent->StartDeepBreathing();
			return;
		case EWellnessBlockType::DeepBreathing:
			DeepBreathingComponent->StartDeepBreathing();
			return;
		}
	}
	// Stop Activity Logic
	else
	{
		// Determine which activity is currently active and stop it.
		switch (MinuteGoalActionsState)
		{
		case EMinuteGoalActionsState::Meditating:
			MeditationComponent->StopMeditation();
			return;
		case EMinuteGoalActionsState::Stretching:
			StretchingComponent->StopStretching();
			return;
		case EMinuteGoalActionsState::DeepBreathing:
			DeepBreathingComponent->StopDeepBreathing();
			return;
		case EMinuteGoalActionsState::Journaling: // Added Journaling case
			JournalingComponent->StopJournaling(); // Assuming StopJournaling exists
			return;
		case EMinuteGoalActionsState::Idle:
			// Already idle, do nothing.
			return;
		default:
			UE_LOG(LogTemp, Warning, TEXT("AEscapeCharacter::Activity() called to stop with unhandled MinuteGoalActionsState: %d"), static_cast<int>(MinuteGoalActionsState));
			return;
		}
	}
}

/**
 *  Handles character movement based on input (forward/backward, right/left).
 * Movement is only allowed if the character is in the Idle wellness state.
 *  Value The input action value containing the 2D movement vector (X=Right/Left, Y=Forward/Backward).
 */
void AEscapeCharacter::Move(const FInputActionValue& Value)
{
	// Only allow movement if the character is not engaged in a wellness activity.
	if (MinuteGoalActionsState == EMinuteGoalActionsState::Idle)
	{
		// Get the 2D movement input vector.
		const FVector2D MovementVector = Value.Get<FVector2D>();

		// Ensure the character has a valid controller.
		if (Controller != nullptr)
		{
			// Get the controller's rotation, considering only the yaw (horizontal rotation) for movement direction.
			const FRotator Rotation = Controller->GetControlRotation();
			const FRotator YawRotation(0, Rotation.Yaw, 0);

			// Calculate the forward direction based on the controller's yaw.
			const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

			// Calculate the right direction based on the controller's yaw.
			const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

			// Apply movement input along the forward/backward and right/left axes
			AddMovementInput(ForwardDirection, MovementVector.Y);
			AddMovementInput(RightDirection, MovementVector.X);
		}
	}
}

/**
 *  Handles camera look input (mouse or gamepad).
 * Applies yaw and pitch input to the controller.
 *  Value The input action value containing the 2D look vector (X=Yaw, Y=Pitch).
 */
void AEscapeCharacter::Look(const FInputActionValue& Value)
{
	// Get the 2D look input vector.
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	// Ensure the character has a valid controller.
	if (Controller != nullptr)
	{
		// Apply yaw (horizontal) rotation input to the controller.
		AddControllerYawInput(LookAxisVector.X);
		// Apply pitch (vertical) rotation input to the controller.
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

/**
 *  Overrides the default ACharacter::Jump function.
 * Initiates the jump action only if the character is in the Idle wellness state.
 */
void AEscapeCharacter::Jump()
{
	// Only allow jumping if not engaged in a wellness activity.
	if (MinuteGoalActionsState == EMinuteGoalActionsState::Idle)
	{
		Super::Jump();
	}
}

/**
 *  Overrides the default ACharacter::StopJumping function.
 * Calls the base class implementation to handle stopping the jump (e.g., reducing Z velocity if jump key released early).
 */
void AEscapeCharacter::StopJumping()
{
	Super::StopJumping();
}

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
#include "Components/MeditationComponent.h"
#include "InputActionValue.h"
#include "Blueprint/UserWidget.h"
#include "Components/SecondCounterComponent.h"
#include "Widgets/MobileUIWidget.h"
#include "Widgets/MessageWidget.h"
#include "Components/WellnessComponent.h"
#include "Components/StretchingComponent.h"
#include "Components/DeepBreathingComponent.h"
#include "Kismet/GameplayStatics.h"

//////////////////////////////////////////////////////////////////////////
// AEscapeCharacter

AEscapeCharacter::AEscapeCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Create and attach the meditation component to this character
	MeditationComponent = CreateDefaultSubobject<UMeditationComponent>(TEXT("MeditationComponent"));
	DeepBreathingComponent = CreateDefaultSubobject<UDeepBreathingComponent>(TEXT("DeepBreathingComponent"));
	StretchingComponent = CreateDefaultSubobject<UStretchingComponent>(TEXT("StretchingComponent"));
	WellnessComponent = CreateDefaultSubobject<UWellnessComponent>(TEXT("WellnessComponent"));
	SecondCounterMeditation = CreateDefaultSubobject<USecondCounterComponent>(TEXT("SecondCounterMeditation"));
	SecondCounterStretching = CreateDefaultSubobject<USecondCounterComponent>(TEXT("SecondCounterStretching"));
	SecondCounterBreathing = CreateDefaultSubobject<USecondCounterComponent>(TEXT("SecondCounterDeepBreathing"));

	// Define the save slot and user index
	SecondCounterMeditation->SaveSlotName = TEXT("MeditationScore");
	SecondCounterMeditation->UserIndex = 0;

	// Define the save slot and user index
	SecondCounterBreathing->SaveSlotName = TEXT("BreathingScore");
	SecondCounterBreathing->UserIndex = 0;

	// Define the save slot and user index
	SecondCounterStretching->SaveSlotName = TEXT("StretchingScore");
	SecondCounterStretching->UserIndex = 0;

	// Initializing the input action as null
	MinuteGoalActions = nullptr;
	MinuteGoalActionsState = EMinuteGoalActionsState::Idle;
	// Initialize widget pointers to null; they are created dynamically
	MobileUIWidget = nullptr;
	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
	SetWellnessBlockType(EWellnessBlockType::None);

}
void AEscapeCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AEscapeCharacter::BeginPlay() {
	Super::BeginPlay();
	
	if (PLATFORM_ANDROID || PLATFORM_IOS) {
		MobileUIWidget = CreateWidget<UMobileUIWidget>(GetWorld(), MobileUIWidgetClass);
		MobileUIWidget->AddToViewport();
		MobileUIWidget->SetPlayer(this);
		MobileUIWidget->InteractionWidget->SetPlayer(Cast<ACharacter>(this));
		MobileUIWidget->ChangeInteractionUI();
	}
	ActivityUIWidget = CreateWidget<UActivityUIWidget>(GetWorld(), ActivityUIWidgetClass);
	ActivityUIWidget->AddToViewport();
	ActivityUIWidget->SetVisibility(ESlateVisibility::Collapsed);
	SecondCounterBreathing->SetScoreWidget(ActivityUIWidget->GetScoreWidget());
	SecondCounterMeditation->SetScoreWidget(ActivityUIWidget->GetScoreWidget());
	SecondCounterStretching->SetScoreWidget(ActivityUIWidget->GetScoreWidget());
	ActivityUIWidget->GetDeepBreathingWidget()->SetPlayer(this);
}
//////////////////////////////////////////////////////////////////////////
// Input
void AEscapeCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {

		// If the MinuteGoalActions component is set, bind the media actions
		if (MinuteGoalActions)
		{
			// Bind the "Started" trigger (e.g., key press or touch) to start activity
			EnhancedInputComponent->BindAction(MinuteGoalActions, ETriggerEvent::Started, this, &AEscapeCharacter::Activity);
		}
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AEscapeCharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AEscapeCharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AEscapeCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AEscapeCharacter::Look);
	}
}



void AEscapeCharacter::Activity()
{
	if (MinuteGoalActionsState == EMinuteGoalActionsState::Idle && !GetCharacterMovement()->IsFalling()) {
		UE_LOG(LogTemp, Warning, TEXT("No affirmations defined in EscapeCharacter "));

		// Start the wellness activity based on the block type
		switch (BlockTypePlayer) {
		case EWellnessBlockType::Meditation:
			// Start meditation
			MeditationComponent->StartMeditation();
			return;
		case EWellnessBlockType::Stretching:
			// Start stretching
			StretchingComponent->StartStretching();
			return;
		case EWellnessBlockType::None:
			// Start Deep Breathing
			DeepBreathingComponent->StartDeepBreathing();
			return;
		}
	
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("affirmations defined in EscapeCharacter "));

		switch (MinuteGoalActionsState) {
		case EMinuteGoalActionsState::Meditating:
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), AWellnessBlock::StaticClass(), WellnessBlc);
			for (AActor* Block : WellnessBlc)
			{
				if (Cast<AWellnessBlock>(Block) && Cast<AWellnessBlock>(Block)->PlayerRef ==this) {
					Cast<AWellnessBlock>(Block)->SetMeditationBlockState(EMeditationBlockState::Lowering);
					return;
				}

			}
		case EMinuteGoalActionsState::Stretching:
			// Start stretching
			StretchingComponent->StopStretching();
			return;
		case EMinuteGoalActionsState::DeepBreathing:
			// Start Deep Breathing
			//DeepBreathingComponent->StopDeepBreathing();
			return;
		}
	}
}



void AEscapeCharacter::Move(const FInputActionValue& Value)
{
	// Checks if the meditationstate is idle in order to move
	if (MinuteGoalActionsState == EMinuteGoalActionsState::Idle) {
		// input is a Vector2D
		FVector2D MovementVector = Value.Get<FVector2D>();

		if (Controller != nullptr)
		{
			// find out which way is forward
			const FRotator Rotation = Controller->GetControlRotation();
			const FRotator YawRotation(0, Rotation.Yaw, 0);

			// get forward vector
			const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

			// get right vector 
			const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

			// add movement 
			AddMovementInput(ForwardDirection, MovementVector.Y);
			AddMovementInput(RightDirection, MovementVector.X);
		}
	}
}

void AEscapeCharacter::Jump()
{
	// Checks if the meditation state is idle in order to jump
	if (MinuteGoalActionsState == EMinuteGoalActionsState::Idle) {
		bPressedJump = true;
		JumpKeyHoldTime = 0.0f;
	}
}

void AEscapeCharacter::StopJumping()
{
	bPressedJump = false;
	ResetJumpState();
}
void AEscapeCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AEscapeCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}




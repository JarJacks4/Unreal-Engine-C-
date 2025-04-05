#include "WellnessBlock.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/MeditationComponent.h"
#include "EscapeCharacter.h"

AWellnessBlock::AWellnessBlock()
{
    // Enable ticking for levitation updates
    PrimaryActorTick.bCanEverTick = true;

    // Create and set up the mesh component as the root
    SceneRoot = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Root"));

    SceneRoot->InitCapsuleSize(42.f, 96.0f);

    BlockMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BlockMesh"));
    RootComponent = SceneRoot;
    BlockMesh->SetupAttachment(RootComponent);
    // Create and set up the trigger volume for overlap detection
    TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
    TriggerVolume->SetupAttachment(BlockMesh);
    TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &AWellnessBlock::OnOverlapBegin);
    TriggerVolume->OnComponentEndOverlap.AddDynamic(this, &AWellnessBlock::OnOverlapEnd);
    // Initialize levitation offset
    LevitationOffset = 0.0f;
}

void AWellnessBlock::BeginPlay()
{
    Super::BeginPlay();
    if (LowerSpeed <= 0) {
        LowerSpeed = 1;
    }
    if (RiseSpeed <= 0) {
        RiseSpeed = 1;
    }
}

void AWellnessBlock::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (PlayerRef != nullptr) {
        UpdateLevitation(DeltaTime);
    }
}

void AWellnessBlock::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    // Check if the overlapping actor is the player character
    if (Cast<AEscapeCharacter>(OtherActor))
    {
        PlayerRef = Cast<ACharacter>(OtherActor);
        // Show the interaction UI based on the block type
        switch (BlockType) {
        case EWellnessBlockType::Meditation:
            MeditationBlockState = EMeditationBlockState::Rising;
            Cast<AEscapeCharacter>(PlayerRef)->SetWellnessBlockType(EWellnessBlockType::Meditation);
            break;
        case EWellnessBlockType::Stretching:
            Cast<AEscapeCharacter>(PlayerRef)->SetWellnessBlockType(EWellnessBlockType::Stretching);
            break;
        }
        Cast<AEscapeCharacter>(PlayerRef)->GetMobileUIWidget()->ChangeInteractionUI();
    }
}

void AWellnessBlock::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    // Check if the actor leaving the overlap is the player character
    if (Cast<AEscapeCharacter>(OtherActor))
    {
        PlayerRef = Cast<ACharacter>(OtherActor);
        Cast<AEscapeCharacter>(PlayerRef)->SetWellnessBlockType(EWellnessBlockType::None);
        // Updates the interaction UI
        Cast<AEscapeCharacter>(PlayerRef)->GetMobileUIWidget()->ChangeInteractionUI();
        PlayerRef = nullptr;
    }
}

void AWellnessBlock::UpdateLevitation(float DeltaTime)
{
    
    if (Cast<AEscapeCharacter>(PlayerRef)->GetMinuteGoalActionsState() == EMinuteGoalActionsState::Meditating && (MeditationBlockState != EMeditationBlockState::None && MeditationBlockState != EMeditationBlockState::Lowering)) {
        if (MeditationBlockState == EMeditationBlockState::Rising) {
            if (BlockMesh->GetRelativeLocation().Z < MaxDistanceFromGround) {
                LevitationOffset = DeltaTime * MovementSpeed * (RiseSpeed / 100);
                BlockMesh->AddLocalOffset(FVector(0, 0, LevitationOffset));
                return;
            }
            else
            {
                LevitationOffset = 0;
                MeditationBlockState = EMeditationBlockState::FloatingDown;
                return;
            }
        }
        else if (MeditationBlockState == EMeditationBlockState::FloatingDown) {
            if (BlockMesh->GetRelativeLocation().Z > LowestDistanceFromGround) {
                LevitationOffset = DeltaTime * -MovementSpeed * (LowerSpeed / 100);
                BlockMesh->AddLocalOffset(FVector(0, 0, LevitationOffset));
                return;
            }
            else {
                LevitationOffset = 0;
                MeditationBlockState = EMeditationBlockState::FloatingUp;
                return;
            }
        }
        else if (MeditationBlockState == EMeditationBlockState::FloatingUp) {
            if (BlockMesh->GetRelativeLocation().Z < MaxDistanceFromGround) {
                LevitationOffset = DeltaTime * MovementSpeed * (RiseSpeed / 100);
                BlockMesh->AddLocalOffset(FVector(0, 0, LevitationOffset));
                return;
            }
            else {
                LevitationOffset = 0;
                MeditationBlockState = EMeditationBlockState::FloatingDown;
                return;
            }
        }
    }
    else if (BlockMesh->GetRelativeLocation().Z >= 1 && Cast<AEscapeCharacter>(PlayerRef)->GetMinuteGoalActionsState() == EMinuteGoalActionsState::Meditating) {
        LevitationOffset = DeltaTime * -MovementSpeed * (LowerSpeed / 100);
        BlockMesh->AddLocalOffset(FVector(0, 0, LevitationOffset));
        
    }
    else {
        if (BlockMesh->GetRelativeLocation().Z < 1) {
            BlockMesh->SetRelativeLocation(FVector(0, 0, 0));
            Cast<AEscapeCharacter>(PlayerRef)->MeditationComponent->StopMeditation();
            MeditationBlockState = EMeditationBlockState::Rising;
        }
    }
    

}
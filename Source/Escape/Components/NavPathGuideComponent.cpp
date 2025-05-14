#include "NavPathGuideComponent.h"
#include "GameFramework/Actor.h"
#include "NavigationSystem.h"
#include "NavMesh/NavMeshPath.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "../EscapeCharacter.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Kismet/KismetSystemLibrary.h" 
/**
 * Constructor for UNavPathGuideComponent
 * Sets default values and configures the component for ticking
 */
UNavPathGuideComponent::UNavPathGuideComponent()
{
    // This component needs to tick to update path visuals for animated path types
    PrimaryComponentTick.bCanEverTick = true;    
    // Initialize properties
    PathDestination = FVector::ZeroVector;
    LastPlayerLocation = FVector::ZeroVector;
    bHasActivePath = false;
    
    // By default, look for a simple cylindrical mesh for path visualization
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMeshFinder(TEXT("/Engine/BasicShapes/Cylinder"));
    if (CylinderMeshFinder.Succeeded())
    {
        PathMesh = CylinderMeshFinder.Object;
    }
    
    // Also try to find an arrow mesh for detailed path mode
    static ConstructorHelpers::FObjectFinder<UStaticMesh> ArrowMeshFinder(TEXT("/Engine/BasicShapes/Cone"));
    if (ArrowMeshFinder.Succeeded())
    {
        ArrowMesh = ArrowMeshFinder.Object;
    }
    
    // Look for a material that we know works well with dynamic color changes
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> DefaultEmissiveMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
    if (DefaultEmissiveMaterial.Succeeded())
    {
        PathMaterial = DefaultEmissiveMaterial.Object;
    }
}

/**
 * Called when the component is initialized
 * Caches reference to the owning character and sets up initial state
 */
void UNavPathGuideComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Cache reference to the owning EscapeCharacter
    CachedEscapeCharacter = Cast<AEscapeCharacter>(GetOwner());
    
    // Initialize starting position to track movement
    if (CachedEscapeCharacter)
    {
        LastPlayerLocation = CachedEscapeCharacter->GetActorLocation();
    }
}

/**
 * Called when the component is destroyed or when the game ends
 * Cleans up any active paths and timers
 */
void UNavPathGuideComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Clear any existing path and cleanup
    ClearPath();
    
    // Cancel any pending timers
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(UpdatePathTimerHandle);
    }
    
    Super::EndPlay(EndPlayReason);
}

/**
 * Called every tick when enabled
 * Updates animated path visuals and handles other per-frame updates
 */
void UNavPathGuideComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Always update the path if auto update is enabled
    if (bAutoUpdatePath)
    {
        UpdatePathIfNeeded();
    }
    // Only process tick if we have an active path and the animated style is selected
    if (bHasActivePath && PathVisualType == EPathVisualType::Animated)
    {
        // For animated path type, update visuals with pulsing or flowing effect
        static float AnimTime = 0.0f;
        AnimTime += DeltaTime;
        
    }
}

/**
 * Ensures the spline component exists, creating it if necessary
 */
void UNavPathGuideComponent::EnsureSplineExists()
{
    if (!PathSpline && GetOwner())
    {
        // Create the spline component
        PathSpline = NewObject<USplineComponent>(GetOwner(), USplineComponent::StaticClass(), TEXT("PathSpline"));
        
        // Set mobility to Movable before registering
        PathSpline->SetMobility(EComponentMobility::Movable);
        
        PathSpline->RegisterComponent();
        
        // Set spline properties
        PathSpline->ClearSplinePoints();
        PathSpline->SetClosedLoop(false);
        
        // Attach to owner's root component
        if (GetOwner()->GetRootComponent())
        {
            PathSpline->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
        }
    }
}

/**
 * Helper function to clamp a point above the ground
 */
FVector UNavPathGuideComponent::ProjectPointToGround(const FVector& Point, float TraceDistanceOverride, float OffsetAboveGround) const
{
    float TraceDist = (TraceDistanceOverride > 0.0f) ? TraceDistanceOverride : TraceDistance;
    FVector Start = Point + FVector(0, 0, TraceDist * 0.5f);
    FVector End = Point - FVector(0, 0, TraceDist * 0.5f);
    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(GetOwner());
    bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params);
    if (!bHit)
    {
        // Try a longer downward trace from higher up if initial trace fails
        FVector HighStart = Point + FVector(0, 0, TraceDist);
        FVector FarEnd = Point - FVector(0, 0, TraceDist * 2.0f);
        bHit = GetWorld()->LineTraceSingleByChannel(HitResult, HighStart, FarEnd, ECC_Visibility, Params);
    }
    // Optional: Draw debug line for troubleshooting
    //#define NAVPATHGUIDE_DEBUG
#ifdef NAVPATHGUIDE_DEBUG
    DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 2.0f, 0, 2.0f);
    if (!bHit) DrawDebugLine(GetWorld(), HighStart, FarEnd, FColor::Red, false, 2.0f, 0, 2.0f);
#endif
    if (bHit)
    {
        return HitResult.ImpactPoint + FVector(0, 0, OffsetAboveGround);
    }
    // Fallback: try to find the lowest point below (e.g., for holes)
    FVector DeepStart = Point + FVector(0, 0, 10.0f);
    FVector DeepEnd = Point - FVector(0, 0, 100000.0f);
    if (GetWorld()->LineTraceSingleByChannel(HitResult, DeepStart, DeepEnd, ECC_Visibility, Params))
    {
        return HitResult.ImpactPoint + FVector(0, 0, OffsetAboveGround);
    }
    return Point; // fallback if no ground found
}

/**
 * Generates a path to the specified world location
 */
bool UNavPathGuideComponent::GeneratePathToLocation(const FVector& Destination)
{
    // Clear any existing path
    ClearPath();
    
    // Store the destination for potential path updates
    PathDestination = Destination;
    
    // Get the navigation system
    UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (!NavSystem || !GetOwner())
    {
        return false;
    }
    
    // Get the starting position (typically the player's current location)
    FVector StartLocation = GetOwner()->GetActorLocation();
    LastPlayerLocation = StartLocation; // Remember where we started
    
    // Create a new navigation path
    CurrentPath = UNavigationSystemV1::FindPathToLocationSynchronously(
        GetWorld(),
        StartLocation,
        PathDestination,
        GetOwner()
    );
    
    // Check if we found a valid path
    if (CurrentPath && CurrentPath->IsValid() && CurrentPath->GetPathLength() > 0)
    {
        // Ensure our spline component exists
        EnsureSplineExists();
        
        // Update the spline points based on the path
        if (PathSpline)
        {
            PathSpline->ClearSplinePoints();
            
            // Get the original path points
            const TArray<FNavPathPoint>& PathPoints = CurrentPath->GetPath()->GetPathPoints();
            TArray<FVector> ProcessedPoints;
            FVector PlayerCenter = GetOwner()->GetActorLocation();
            // Only add player center if navmesh start is not close
            if (PathPoints.Num() > 0 && FVector::Dist(PlayerCenter, PathPoints[0].Location) > 10.0f)
            {
                ProcessedPoints.Add(PlayerCenter);
            }
            // Add navmesh path points, skipping duplicates/nearby
            for (int32 i = 0; i < PathPoints.Num(); i++)
            {
                FVector CurrentPoint = PathPoints[i].Location;
                if (ProcessedPoints.Num() == 0 || FVector::Dist(CurrentPoint, ProcessedPoints.Last()) > 1.0f)
                {
                    ProcessedPoints.Add(CurrentPoint);
                }
            }
            float MinPointDistance = PathWidth * 1.0f;
            // Remove points that are too close (for visual smoothness)
            for (int32 i = ProcessedPoints.Num() - 2; i >= 0; --i)
            {
                if (FVector::Dist(ProcessedPoints[i], ProcessedPoints[i+1]) < 1.0f)
                {
                    ProcessedPoints.RemoveAt(i);
                }
            }
            // Subdivide segments so that no segment exceeds MaxSplineSegmentLength
            TArray<FVector> SubdividedPoints;
            if (ProcessedPoints.Num() > 0)
            {
                SubdividedPoints.Add(ProjectPointToGround(ProcessedPoints[0], TraceDistance, PathHeightOffset));
                for (int32 i = 1; i < ProcessedPoints.Num(); ++i)
                {
                    FVector Start = ProcessedPoints[i - 1];
                    FVector End = ProcessedPoints[i];
                    float SegmentLength = FVector::Dist(Start, End);
                    int32 NumSegments = FMath::CeilToInt(SegmentLength / MaxSplineSegmentLength);
                    for (int32 s = 1; s <= NumSegments; ++s)
                    {
                        float Alpha = float(s) / float(NumSegments);
                        FVector Point = FMath::Lerp(Start, End, Alpha);
                        Point = ProjectPointToGround(Point, TraceDistance, PathHeightOffset);
                        if (FVector::Dist(Point, SubdividedPoints.Last()) > 1.0f)
                        {
                            SubdividedPoints.Add(Point);
                        }
                    }
                }
            }
            
            // Add the subdivided points to the spline
            // Remove any duplicate or near-duplicate points at the start (fixes disappearing spline near actor)
            for (int32 i = 0; i < SubdividedPoints.Num(); ++i)
            {
                if (i == 0 || FVector::Dist(SubdividedPoints[i], SubdividedPoints[i-1]) > 1.0f)
                {
                    PathSpline->AddSplinePoint(SubdividedPoints[i], ESplineCoordinateSpace::World);
                }
            }
            
            // Update the visual representation of the path
            UpdatePathVisuals();
            bHasActivePath = true;
            
            return true;
        }
    }
    
    return false;
}

/**
 * Generates a path to the specified actor
 */
bool UNavPathGuideComponent::GeneratePathToActor(AActor* TargetActor)
{
    if (TargetActor)
    {
        return GeneratePathToLocation(TargetActor->GetActorLocation());
    }
    return false;
}

/**
 * Updates the visual representation of the path
 */
void UNavPathGuideComponent::UpdatePathVisuals()
{
    // Ensure we have a spline and mesh
    if (!PathSpline || !PathMesh)
    {
        return;
    }
    
    // Clear any existing spline meshes
    for (USplineMeshComponent* SplineMesh : SplineMeshes)
    {
        if (SplineMesh)
        {
            SplineMesh->DestroyComponent();
        }
    }
    SplineMeshes.Empty();
    
    // We'll use the PathMaterial directly instead of creating a dynamic instance
    if (!SharedDynMat && PathMaterial)
    {
        SharedDynMat = UMaterialInstanceDynamic::Create(PathMaterial, this);
        if (SharedDynMat)
        {
            SharedDynMat->SetVectorParameterValue(PathColorParameterName, PathColor);
            SharedDynMat->SetScalarParameterValue(TEXT("Opacity"), 1.0f);
        }
    }
    
    // Check if the guide should be shown
    if (!bShowNavGuide)
    {
        // Hide all spline meshes if the guide is toggled off
        for (USplineMeshComponent* SplineMesh : SplineMeshes)
        {
            if (SplineMesh)
            {
                SplineMesh->SetVisibility(false);
            }
        }
        if (PathSpline)
        {
            PathSpline->SetVisibility(false);
        }
        return;
    }
    else
    {
        for (USplineMeshComponent* SplineMesh : SplineMeshes)
        {
            if (SplineMesh)
            {
                SplineMesh->SetVisibility(true);
            }
        }
        if (PathSpline)
        {
            PathSpline->SetVisibility(true);
        }
    }
    
    // Get number of spline points
    int32 NumPoints = PathSpline->GetNumberOfSplinePoints();
    if (NumPoints < 2)
    {
        return; // Need at least 2 points for a path segment
    }

    // Control the maximum segment length for spline meshes
    float MaxSegmentLength = MaxSplineSegmentLength; // Use the UPROPERTY from the header
    int32 StartIndex = 0;
    while (StartIndex < NumPoints - 1)
    {
        float SegmentLength = 0.0f;
        int32 EndIndex = StartIndex + 1;
        // Find the furthest point within MaxSegmentLength
        while (EndIndex < NumPoints)
        {
            SegmentLength = PathSpline->GetDistanceAlongSplineAtSplinePoint(EndIndex) - PathSpline->GetDistanceAlongSplineAtSplinePoint(StartIndex);
            if (SegmentLength > MaxSegmentLength)
            {
                EndIndex--;
                break;
            }
            EndIndex++;
        }
        if (EndIndex >= NumPoints) EndIndex = NumPoints - 1;
        if (EndIndex == StartIndex) EndIndex = StartIndex + 1;

        UpdateSplineMesh(StartIndex);
        StartIndex = EndIndex;
    }
}

/**
 * Creates or updates a spline mesh at the given index
 */
void UNavPathGuideComponent::UpdateSplineMesh(int32 SegmentIndex)
{
    if (!PathSpline || !PathMesh) return;
    if (SegmentIndex < 0 || SegmentIndex >= PathSpline->GetNumberOfSplinePoints() - 1) return;

    FVector StartPos = PathSpline->GetLocationAtSplinePoint(SegmentIndex, ESplineCoordinateSpace::World);
    FVector EndPos = PathSpline->GetLocationAtSplinePoint(SegmentIndex + 1, ESplineCoordinateSpace::World);
    FVector StartTangent = PathSpline->GetTangentAtSplinePoint(SegmentIndex, ESplineCoordinateSpace::World);
    FVector EndTangent = PathSpline->GetTangentAtSplinePoint(SegmentIndex + 1, ESplineCoordinateSpace::World);

    // Snap both start and end to ground
    StartPos = ProjectPointToGround(StartPos, TraceDistance, PathHeightOffset);
    EndPos = ProjectPointToGround(EndPos, TraceDistance, PathHeightOffset);

    // Recalculate tangents to follow the ground exactly
    FVector SegmentDirection = (EndPos - StartPos).GetSafeNormal();
    float StartTangentLength = (PathSpline->GetTangentAtSplinePoint(SegmentIndex, ESplineCoordinateSpace::World)).Size();
    float EndTangentLength = (PathSpline->GetTangentAtSplinePoint(SegmentIndex + 1, ESplineCoordinateSpace::World)).Size();
    StartTangent = SegmentDirection * StartTangentLength;
    EndTangent = SegmentDirection * EndTangentLength;

    USplineMeshComponent* SplineMesh = nullptr;
    if (SegmentIndex < SplineMeshes.Num())
    {
        SplineMesh = SplineMeshes[SegmentIndex];
    }
    else
    {
        SplineMesh = NewObject<USplineMeshComponent>(GetOwner());
        SplineMesh->SetMobility(EComponentMobility::Movable);
        SplineMesh->SetStaticMesh(PathMesh);
        SplineMesh->RegisterComponent();
        SplineMeshes.Add(SplineMesh);
    }

    // Set the width/scale of the spline mesh based on PathWidth, but use a much smaller multiplier for better visual fit
    float WidthScale = PathWidth * 0.04f; // Make the path even thinner
    FVector2D MeshScale(WidthScale, WidthScale);
    SplineMesh->SetStartScale(MeshScale);
    SplineMesh->SetEndScale(MeshScale);
    // Center the mesh by offsetting it along the right vector
    FVector MeshOffset = FVector::ZeroVector;
    if (PathMesh)
    {
        FVector MeshBoundsOrigin, MeshBoundsExtent;
        PathMesh->GetBounds().GetBox().GetCenterAndExtents(MeshBoundsOrigin, MeshBoundsExtent);
        MeshOffset = -PathSpline->GetRightVectorAtSplinePoint(SegmentIndex, ESplineCoordinateSpace::World) * MeshBoundsExtent.Y * MeshScale.Y;
    }
    SplineMesh->SetStartAndEnd(StartPos + MeshOffset, StartTangent, EndPos + MeshOffset, EndTangent);

    // Assign the shared dynamic material instance and set color
    if (SharedDynMat)
    {
        SplineMesh->SetMaterial(0, SharedDynMat);
        SplineMesh->SetRenderCustomDepth(true);
        SplineMesh->SetCustomDepthStencilValue(252);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[NavPathGuide] SharedDynMat is null. Spline mesh will have no material assigned!"));
    }
    SplineMesh->SetRenderCustomDepth(true);
    SplineMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

/**
 * Clears the current path and its visual representation
 */
void UNavPathGuideComponent::ClearPath()
{
    // Clear the spline points
    if (PathSpline)
    {
        PathSpline->ClearSplinePoints();
    }
    
    // Destroy all spline mesh components
    for (USplineMeshComponent* SplineMesh : SplineMeshes)
    {
        if (SplineMesh)
        {
            SplineMesh->DestroyComponent();
        }
    }
    SplineMeshes.Empty();
    
    // Reset path state (do NOT reset PathDestination)
    CurrentPath = nullptr;
    bHasActivePath = false;
    // PathDestination is intentionally NOT reset here, so the guide can regenerate when returning to navmesh
}

/**
 * Sets the path visual style
 */
void UNavPathGuideComponent::SetPathVisualType(EPathVisualType NewVisualType)
{
    if (PathVisualType != NewVisualType)
    {
        PathVisualType = NewVisualType;
        
        // Update visuals if we have an active path
        if (bHasActivePath)
        {
            UpdatePathVisuals();
        }
    }
}

/**
 * Sets the path color
 */
void UNavPathGuideComponent::SetPathColor(const FLinearColor& NewColor)
{
    PathColor = NewColor;
    // Update all spline mesh dynamic materials
    for (USplineMeshComponent* SplineMesh : SplineMeshes)
    {
        if (SplineMesh)
        {
            UMaterialInstanceDynamic* DynMat = Cast<UMaterialInstanceDynamic>(SplineMesh->GetMaterial(0));
            if (DynMat)
            {
                DynMat->SetVectorParameterValue(PathColorParameterName, PathColor);
            }
        }
    }
    // Update Material Parameter Collection if set
    if (PathMaterialParameterCollection)
    {
        UWorld* World = GetWorld();
        if (World)
        {
            UMaterialParameterCollectionInstance* MPCInst = World->GetParameterCollectionInstance(PathMaterialParameterCollection);
            if (MPCInst)
            {
                MPCInst->SetVectorParameterValue(PathColorParameterName, PathColor);
            }
        }
    }
}

/**
 * Updates the path if the player has moved more than the update threshold
 */
void UNavPathGuideComponent::UpdatePathIfNeeded()
{
    if (!GetOwner())
    {
        return;
    }

    // Check if player is on the navmesh
    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (!NavSys)
    {
        UE_LOG(LogTemp, Warning, TEXT("NavPathGuideComponent: No NavigationSystem found."));
        return;
    }

    FVector OwnerLocation = GetOwner()->GetActorLocation();
    FNavLocation NavLocation;
    bool bOnNavmesh = NavSys->ProjectPointToNavigation(OwnerLocation, NavLocation, FVector(50, 50, 200));

    if (!bOnNavmesh)
    {
        if (bHasActivePath)
        {
            UE_LOG(LogTemp, Log, TEXT("NavPathGuideComponent: Player left navmesh, clearing path."));
            ClearPath();
        }
        return;
    }
    else
    {
        if (!bHasActivePath && PathDestination != FVector::ZeroVector)
        {
            UE_LOG(LogTemp, Log, TEXT("NavPathGuideComponent: Player returned to navmesh, attempting to regenerate path."));
            // Try to regenerate the path
            if (GeneratePathToLocation(PathDestination))
            {
                UE_LOG(LogTemp, Log, TEXT("NavPathGuideComponent: Path successfully rebuilt."));
                // Update LastPlayerLocation so the path is not immediately cleared again
                LastPlayerLocation = OwnerLocation;
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("NavPathGuideComponent: Failed to rebuild path after returning to navmesh."));
            }
        }
    }

    // Check if player has moved enough to warrant an update
    float DistanceSquared = FVector::DistSquared(OwnerLocation, LastPlayerLocation);
    if (bHasActivePath && DistanceSquared > (UpdatePathThreshold * UpdatePathThreshold))
    {
        GeneratePathToLocation(PathDestination);
    }
}

/**
 * Enables or disables automatic path updates
 */
void UNavPathGuideComponent::EnableAutomaticUpdates(bool bEnable)
{
    bAutoUpdatePath = bEnable;
    
    // Get world to access timer manager
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Clear any existing timer
    World->GetTimerManager().ClearTimer(UpdatePathTimerHandle);
    
    // Create new timer if enabled
    if (bAutoUpdatePath && bHasActivePath)
    {
        // Set timer to call UpdatePathIfNeeded as fast as possible (every tick)
        World->GetTimerManager().SetTimer(
            UpdatePathTimerHandle,
            this,
            &UNavPathGuideComponent::UpdatePathIfNeeded,
            UpdateInterval, // Use property for interval
            true // Loop
        );
    }
}
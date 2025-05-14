#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Components/SplineComponent.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "NavPathGuideComponent.generated.h"

class AEscapeCharacter;

/**
 * Enum defining different types of path visualization styles.
 */
UENUM(BlueprintType)
enum class EPathVisualType : uint8
{
    Simple      UMETA(DisplayName = "Simple Line"),
    Detailed    UMETA(DisplayName = "Detailed Arrow"),
    Animated    UMETA(DisplayName = "Animated Pulse")
};

/**
 *  UNavPathGuideComponent
 * A component that guides the player to specific locations using navmesh pathfinding
 * and represents the path with splines on the ground.
 * Can dynamically update as the player moves or the destination changes.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ESCAPE_API UNavPathGuideComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    /**  Constructor for the NavPathGuideComponent. */
    UNavPathGuideComponent();

    /**
     *  Generate a path to the specified world location using the nav mesh.
     *  @param Destination The world location to navigate to
     *  @return True if a valid path was found
     */
    UFUNCTION(BlueprintCallable, Category = "Navigation|Path")
    bool GeneratePathToLocation(const FVector& Destination);

    /**
     *  Generate a path to the specified actor using the nav mesh.
     *  @param TargetActor The actor to navigate to
     *  @return True if a valid path was found
     */
    UFUNCTION(BlueprintCallable, Category = "Navigation|Path")
    bool GeneratePathToActor(AActor* TargetActor);
    
    /**
     *  Updates the visual representation of the path with spline meshes.
     */
    UFUNCTION(BlueprintCallable, Category = "Navigation|Visuals")
    void UpdatePathVisuals();
    
    /**
     *  Clears the current path and its visual representation.
     */
    UFUNCTION(BlueprintCallable, Category = "Navigation|Path")
    void ClearPath();
    
    /**
     *  Sets the path visual style.
     *  @param NewVisualType The new visual style to use for the path
     */
    UFUNCTION(BlueprintCallable, Category = "Navigation|Visuals")
    void SetPathVisualType(EPathVisualType NewVisualType);
    
    /**
     *  Gets the current path visual style.
     *  @return The current visual style in use
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Navigation|Visuals")
    EPathVisualType GetPathVisualType() const { return PathVisualType; }
    
    /**
     *  Sets the path color.
     *  @param NewColor The new color for the path visuals
     */
    UFUNCTION(BlueprintCallable, Category = "Navigation|Visuals")
    void SetPathColor(const FLinearColor& NewColor);
    
    /**
     *  Gets the current path color.
     *  @return The current color of the path visuals
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Navigation|Visuals")
    FLinearColor GetPathColor() const { return PathColor; }

    /**
     *  Updates the path if the player has moved more than the update threshold.
     *  Called manually or on a timer.
     */
    UFUNCTION(BlueprintCallable, Category = "Navigation|Path")
    void UpdatePathIfNeeded();

    /**
     *  Enables automatic path updates on a timer.
     *  @param bEnable Whether to enable automatic updates
     */
    UFUNCTION(BlueprintCallable, Category = "Navigation|Path")
    void EnableAutomaticUpdates(bool bEnable);

    /**
     *  Projects a point to the ground using a trace.
     *  @param Point The point to project
     *  @param TraceDistanceOverride The maximum trace distance override
     *  @param OffsetAboveGround The offset above the ground
     *  @return The projected point
     */
    FVector ProjectPointToGround(const FVector& Point, float TraceDistanceOverride = -1.0f, float OffsetAboveGround = 5.0f) const;

    /**
     *  Maximum length of each spline mesh segment (smaller = more segments, smoother path)
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation|Path")
    float MaxSplineSegmentLength = 75.0f;

    /**
     *  The maximum trace distance for projecting spline points to the ground.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation|Visuals")
    float TraceDistance = 10000.0f;

    /**
     *  Property to toggle the guide.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation|Path")
    bool bShowNavGuide = true;

protected:
    /** Called when the game starts */
    virtual void BeginPlay() override;
    
    /** Called when the component is destroyed */
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    
    /** Called every frame */
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    /**
     *  The main spline component used to represent the path.
     *  Created dynamically in BeginPlay.
     */
    UPROPERTY(Transient)
    TObjectPtr<USplineComponent> PathSpline;
    
    /**
     *  Array of spline mesh components used to visualize the path.
     *  Created and updated dynamically.
     */
    UPROPERTY(Transient)
    TArray<TObjectPtr<USplineMeshComponent>> SplineMeshes;
    
    /**
     *  The current navigation path as calculated by the navigation system.
     */
    UPROPERTY(Transient)
    TObjectPtr<UNavigationPath> CurrentPath;
    
    /**
     *  The destination world location for the current path.
     */
    UPROPERTY(Transient)
    FVector PathDestination;
    
    /**
     *  The last location of the player that was used to generate the path.
     *  Used to determine when to update the path.
     */
    UPROPERTY(Transient)
    FVector LastPlayerLocation;
    
    /**
     *  The visual appearance style for the path.
     */
    UPROPERTY(EditAnywhere, Category = "Navigation|Visuals")
    EPathVisualType PathVisualType = EPathVisualType::Simple;
    
    /**
     *  The color of the path visuals.
     */
    UPROPERTY(EditAnywhere, Category = "Navigation|Visuals")
    FLinearColor PathColor = FLinearColor(0.0f, 0.75f, 1.0f, 1.0f); // Light blue by default
    
    /**
     *  The static mesh to use for the path visuals.
     */
    UPROPERTY(EditAnywhere, Category = "Navigation|Visuals")
    TObjectPtr<UStaticMesh> PathMesh;

    /**
     *  The static mesh to use for arrow segments in detailed path mode.
     */
    UPROPERTY(EditAnywhere, Category = "Navigation|Visuals")
    TObjectPtr<UStaticMesh> ArrowMesh;
    
    /**
     *  The material to use for the path visuals.
     */
    UPROPERTY(EditAnywhere, Category = "Navigation|Visuals")
    TObjectPtr<UMaterialInterface> PathMaterial;
    
    /**
     *  The width of the path visuals.
     */
    UPROPERTY(EditAnywhere, Category = "Navigation|Visuals", meta = (ClampMin = "1.0", UIMin = "1.0"))
    float PathWidth = 30.0f;
    
    /**
     *  The offset from the ground for the path visuals.
     */
    UPROPERTY(EditAnywhere, Category = "Navigation|Visuals")
    float PathHeightOffset = 5.0f;
    
    /**
     *  Distance threshold in cm that triggers a path update when the player moves.
     */
    UPROPERTY(EditAnywhere, Category = "Navigation|Path", meta = (EditCondition = "bAutoUpdatePath"))
    float UpdatePathThreshold = 0.0f;
    
    /**
     *  Whether to automatically update the path as the player moves.
     */
    UPROPERTY(EditAnywhere, Category = "Navigation|Path")
    bool bAutoUpdatePath = false;
    
    /**
     *  Interval in seconds between automatic path updates.
     */
    UPROPERTY(EditAnywhere, Category = "Navigation|Path", meta = (ClampMin = "0", UIMin = "0", EditCondition = "bAutoUpdatePath"))
    float UpdateInterval = 0.5f;
    
    /**
     *  Timer handle for automatic path updates.
     */
    FTimerHandle UpdatePathTimerHandle;
    
    /**
     *  Name of the color parameter in the material (default: "Color")
     */
    UPROPERTY(EditAnywhere, Category = "Navigation|Visuals")
    FName PathColorParameterName = TEXT("Color");

    /**
     *  Optional Material Parameter Collection for global color control
     */
    UPROPERTY(EditAnywhere, Category = "Navigation|Visuals")
    TObjectPtr<UMaterialParameterCollection> PathMaterialParameterCollection = nullptr;

    /**
     *  Shared dynamic material instance for all spline meshes
     */
    UPROPERTY(Transient)
    UMaterialInstanceDynamic* SharedDynMat = nullptr;

    /**
     *  Internal method to create the spline component if it doesn't exist.
     */
    void EnsureSplineExists();
    
    /**
     *  Internal method to create or update a spline mesh at the given index.
     *  @param SegmentIndex The index of the spline mesh to update
     */
    void UpdateSplineMesh(int32 SegmentIndex);
    
    /**
     *  Cached reference to the owning character for efficient access.
     */
    UPROPERTY(Transient)
    AEscapeCharacter* CachedEscapeCharacter = nullptr;
    
    /**
     *  Whether the path has been generated and is currently displayed.
     */
    bool bHasActivePath = false;
};
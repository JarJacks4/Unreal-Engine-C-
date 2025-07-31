#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ChatMenuWidget.generated.h"

/**
 * Base class for the Chat Menu Widget
 * Handles null reference issues with LucilleRef and other chat-related components
 */
UCLASS()
class ESCAPE_API UChatMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * Safely gets the Lucille reference with null checks
	 * @return Pointer to Lucille reference, or nullptr if not found
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chat Menu|Safe Access")
	UObject* GetSafeLucilleRef() const;

	/**
	 * Sets the Lucille reference with validation
	 * @param LucilleRef The reference to set
	 */
	UFUNCTION(BlueprintCallable, Category = "Chat Menu|Setup")
	void SetSafeLucilleRef(UObject* LucilleRef);

	/**
	 * Checks if the chat menu is properly initialized
	 * @return True if all required references are valid
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chat Menu|Validation")
	bool IsChatMenuValid() const;

protected:
	/**
	 * Called after the underlying Slate widget is constructed.
	 * Initializes the widget and validates references.
	 */
	virtual void NativeConstruct() override;

private:
	/** Reference to Lucille (can be null) */
	UPROPERTY(Transient)
	TObjectPtr<UObject> LucilleRef;

	/** Flag indicating if the widget is properly initialized */
	bool bIsInitialized = false;
}; 
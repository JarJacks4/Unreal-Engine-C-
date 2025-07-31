#include "ChatMenuWidget.h"

void UChatMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	// Initialize the widget
	bIsInitialized = true;
	
	// Log initialization status
	if (LucilleRef)
	{
		UE_LOG(LogTemp, Log, TEXT("UChatMenuWidget::NativeConstruct: LucilleRef is valid"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UChatMenuWidget::NativeConstruct: LucilleRef is null - this is expected during initialization"));
	}
}

UObject* UChatMenuWidget::GetSafeLucilleRef() const
{
	if (!bIsInitialized)
	{
		UE_LOG(LogTemp, Warning, TEXT("UChatMenuWidget::GetSafeLucilleRef: Widget not initialized"));
		return nullptr;
	}

	if (!LucilleRef)
	{
		UE_LOG(LogTemp, Warning, TEXT("UChatMenuWidget::GetSafeLucilleRef: LucilleRef is null"));
		return nullptr;
	}

	return LucilleRef;
}

void UChatMenuWidget::SetSafeLucilleRef(UObject* NewLucilleRef)
{
	LucilleRef = NewLucilleRef;
	
	if (LucilleRef)
	{
		UE_LOG(LogTemp, Log, TEXT("UChatMenuWidget::SetSafeLucilleRef: LucilleRef set successfully"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UChatMenuWidget::SetSafeLucilleRef: LucilleRef set to null"));
	}
}

bool UChatMenuWidget::IsChatMenuValid() const
{
	if (!bIsInitialized)
	{
		UE_LOG(LogTemp, Warning, TEXT("UChatMenuWidget::IsChatMenuValid: Widget not initialized"));
		return false;
	}

	if (!LucilleRef)
	{
		UE_LOG(LogTemp, Warning, TEXT("UChatMenuWidget::IsChatMenuValid: LucilleRef is null"));
		return false;
	}

	return true;
} 

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Engine/Texture.h"
#include <Components/Button.h>
#include <Components/Image.h>
#include "InteractionWidget.generated.h"


/**
 * 
 */
UCLASS()
class ESCAPE_API UInteractionWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	void SetWidgetMeditationImage();
	UFUNCTION(BlueprintCallable)
	void SetWidgetDeepBreathingImage();
	UFUNCTION(BlueprintCallable)
	void SetWidgetStretchingImage();
	UFUNCTION(BlueprintCallable)
	void SetPlayer(ACharacter* Player) { OwningPlayer = Player; };

	UFUNCTION(BlueprintCallable)
	ACharacter* GetPlayer() { return OwningPlayer; };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction Widget")
	UButton* InteractionWidgetButton;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction Widget")
	TObjectPtr<class UTexture2D> MeditationImage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction Widget")
	TObjectPtr<class UTexture2D> DeepBreathingImage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction Widget")
	TObjectPtr<class UTexture2D> StretchingImage;
private:
	ACharacter* OwningPlayer;
};

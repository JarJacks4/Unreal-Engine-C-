// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "EscapeGameMode.generated.h"

/**
 *  The base Game Mode class for the Escape project.
 * Defines the game rules and defaults, such as the default pawn class.
 * This implementation sets the default pawn to a specific Blueprint character.
 */
UCLASS()
class ESCAPE_API AEscapeGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	/**
	 *  Constructor for AEscapeGameMode.
	 * Sets the DefaultPawnClass by finding a specific Blueprint asset.
	 */
	AEscapeGameMode();
};

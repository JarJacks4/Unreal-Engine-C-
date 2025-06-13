// Copyright Epic Games, Inc. All Rights Reserved.

#include "EscapeGameMode.h"
#include "EscapeCharacter.h" // Include character header, although not directly used here, it's related to the pawn class
#include "UObject/ConstructorHelpers.h" // Required for FClassFinder
#include "GameFramework/Pawn.h" // Required for APawn type

/**
 *  Constructor for the AEscapeGameMode class.
 * Sets the default pawn class to the Blueprint character 'BP_MainChar' found at a specific path.
 * This ensures players spawn using the correct character Blueprint when this game mode is active.
 */
AEscapeGameMode::AEscapeGameMode()
{
	// Define the path to the player character Blueprint asset.
	static const TCHAR* PlayerPawnBPPath = TEXT("/Game/Player/Blueprints/BP_MainChar");

	// Use ConstructorHelpers::FClassFinder to find the Blueprint asset class at the specified path.
	// This helper is specifically designed for use within constructors to load class assets.
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(PlayerPawnBPPath);

	// Check if the Blueprint class was successfully found.
	if (PlayerPawnBPClass.Class != nullptr)
	{
		// If found, set it as the default pawn class for this game mode.
		// When the game starts, the engine will automatically spawn an instance of this pawn for the player.
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

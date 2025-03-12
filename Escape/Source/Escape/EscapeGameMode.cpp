// Copyright Epic Games, Inc. All Rights Reserved.

#include "EscapeGameMode.h"
#include "EscapeCharacter.h"
#include "UObject/ConstructorHelpers.h"

AEscapeGameMode::AEscapeGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

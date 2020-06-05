// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "ActionRPGGameMode.h"
#include "UObject/ConstructorHelpers.h"
#include "Player/RPGPlayerController.h"

AActionRPGGameMode::AActionRPGGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
		PlayerControllerClass = ARPGPlayerController::StaticClass();
	}
}

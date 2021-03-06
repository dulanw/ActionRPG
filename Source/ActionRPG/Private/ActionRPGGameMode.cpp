// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "ActionRPGGameMode.h"
#include "UObject/ConstructorHelpers.h"
#include "Player/RPGPlayerController.h"
#include "Player/RPGPlayerState.h"

AActionRPGGameMode::AActionRPGGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Player/Blueprints/BP_PlayerCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
		PlayerControllerClass = ARPGPlayerController::StaticClass();
		PlayerStateClass = ARPGPlayerState::StaticClass();
	}
}

// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/RPGPlayerController.h"
#include "Character/RPGCharacterBase.h"

void ARPGPlayerController::AcknowledgePossession(class APawn* P)
{
	Super::AcknowledgePossession(P);

	ARPGCharacterBase* CharacterBase = Cast<ARPGCharacterBase>(P);
	if (CharacterBase)
	{
		CharacterBase->GetAbilitySystemComponent()->InitAbilityActorInfo(CharacterBase, CharacterBase);
		CharacterBase->GetAbilitySystemComponent()->RefreshAbilityActorInfo();
	}
}

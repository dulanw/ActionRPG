// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/RPGPlayerController.h"
//#include "Character/RPGCharacterBase.h"

void ARPGPlayerController::AcknowledgePossession(APawn* P)
{
	Super::AcknowledgePossession(P);

/*
	//done in OnRep_Controller()
	ARPGCharacterBase* CharacterBase = Cast<ARPGCharacterBase>(P);
	if (CharacterBase)
	{
		CharacterBase->GetAbilitySystemComponent()->InitAbilityActorInfo(CharacterBase, CharacterBase);
		CharacterBase->GetAbilitySystemComponent()->RefreshAbilityActorInfo();
	}*/
}

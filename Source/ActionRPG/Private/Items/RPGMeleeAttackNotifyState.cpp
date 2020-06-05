// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/RPGMeleeAttackNotifyState.h"
#include "Character/RPGCharacterBase.h"
#include "Items/RPGMeleeWeaponActor.h"

void URPGMeleeAttackNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration);
	ARPGCharacterBase* CharOwner = Cast<ARPGCharacterBase>(MeshComp->GetOwner());
	ARPGMeleeWeaponActor* MeleeWeapon = CharOwner ? Cast<ARPGMeleeWeaponActor>(CharOwner->GetCurrentWeapon()) : nullptr;

	if (MeleeWeapon)
	{
		MeleeWeapon->BeginAttack();
	}
}

void URPGMeleeAttackNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::NotifyEnd(MeshComp, Animation);

	ARPGCharacterBase* CharOwner = Cast<ARPGCharacterBase>(MeshComp->GetOwner());
	ARPGMeleeWeaponActor* MeleeWeapon = CharOwner ? Cast<ARPGMeleeWeaponActor>(CharOwner->GetCurrentWeapon()) : nullptr;

	if (MeleeWeapon)
	{
		MeleeWeapon->EndAttack();
	}
}

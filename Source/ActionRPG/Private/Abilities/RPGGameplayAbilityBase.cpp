// Fill out your copyright notice in the Description page of Project Settings.ActivationOwnedTags


#include "Abilities/RPGGameplayAbilityBase.h"
#include "Items/RPGMeleeWeaponActor.h"
#include "Character/RPGCharacterBase.h"
#include "Animation/AnimMontage.h"

URPGGameplayAbilityBase::URPGGameplayAbilityBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void URPGGameplayAbilityBase::HandleTargetData(const FGameplayAbilityTargetDataHandle& TargetData) const
{
	//return if no target data
	if (TargetData.Num() <= 0)
	{
		return;
	}

	//we will apply the damage effect even if the target has 100% damage immunity, the damage execution calculation will take care of damage block, amour etc..
	if (DamageGameplayEffectClass)
	{
		FGameplayEffectSpecHandle DamageSpecHandle = MakeOutgoingGameplayEffectSpec(DamageGameplayEffectClass, GetAbilityLevel());
		FGameplayEffectSpec* DamageSpec = DamageSpecHandle.Data.Get();

		//#TODO calculate the damage with critical hit chance and damage %
		float DamageMagnitude = 0.0f;
		bool ApplyDamageEffect = CalculateDamageMagnitude(DamageMagnitude);

		//check if attack missed
		if (DamageSpec && ApplyDamageEffect)
		{
			DamageSpec->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.Damage")), DamageMagnitude);
			ApplyGameplayEffectSpecToTarget(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, DamageSpecHandle, TargetData);
		}
	}
	else
	{
		UE_LOG(LogAbilitySystem, Warning, TEXT("Ability %s DamageGameplayEffectClass null"), *GetName());
	}
}

void URPGGameplayAbilityBase::K2_HandleTargetData_Implementation(const FGameplayAbilityTargetDataHandle& TargetData) const
{
	HandleTargetData(TargetData);
}

FGameplayTagContainer URPGGameplayAbilityBase::GetDynamicCooldownTags() const
{
	//#TODO cast to a base weapon/ability actor which will have damages, critical hit chance etc..
	const ARPGAbilityActorBase* SourceActor = Cast<ARPGAbilityActorBase>(GetCurrentSourceObject());
	const ARPGCharacterBase* AvatarCharacter = Cast<ARPGCharacterBase>(GetAvatarActorFromActorInfo());

	if (!SourceActor || !AvatarCharacter)
	{
		//return empty
		return FGameplayTagContainer();
	}

	const FGameplayAbilitySpecHandle SpecHandle = GetCurrentAbilitySpecHandle();
	const ERPGInventorySlot AbilitySlot = AvatarCharacter->GetGameplayAbilityHandleSlot(SpecHandle);
	FGameplayTagContainer CooldownTags = AvatarCharacter->GetSlotCooldownTags(AbilitySlot);
	CooldownTags.AppendTags(SourceActor->GetCooldownTags());

	if (CooldownTags.Num() <= 0)
	{
		UE_LOG(LogAbilitySystem, Warning, TEXT("Ability %s GetDynamicCooldownTags Empty"), *GetName());
	}

	return CooldownTags;
}

float URPGGameplayAbilityBase::GetAnimPlayRate() const
{
	const ARPGAbilityActorBase* SourceActor = Cast<ARPGAbilityActorBase>(GetCurrentSourceObject());
	if (!SourceActor)
	{
		UE_LOG(LogAbilitySystem, Warning, TEXT("URPGGameplayAbilityBase::GetAnimPlayRate: Ability %s GetAnimationPlayRate SourceActor null"), *GetName());
		return 0.0f;
	}

	UAnimMontage* AttackMontage = SourceActor->GetAbilityMontage();
	if (!AttackMontage)
	{
		UE_LOG(LogAbilitySystem, Warning, TEXT("URPGGameplayAbilityBase::GetAnimPlayRate: Ability %s GetAnimationPlayRate GetAbilityMontage returned null"), *GetName());
		return 0.0f;
	}

	//the play time cannot be more than the cool down, we wanna finish anim before we can attack next
	//we take the min from the cool down or attack speed
	const float CooldownDuration = GetCooldownDuration(); //for melee attacks, the cool down is also our attack speed
	const float MaxAnimPlayTime = SourceActor->GetBaseAnimPlayTime();
	
	//we take the min of the 2, we don't want to still be playing the animation and we don't to play an animation at a rate that looks very slow if the
	//cool down is really long
	const float PlayTime = FMath::Min(CooldownDuration, MaxAnimPlayTime);
	if (FMath::IsNearlyZero(PlayTime))
	{
		UE_LOG(LogAbilitySystem, Warning, TEXT("URPGGameplayAbilityBase::GetAnimPlayRate: AbilityActor %s CooldownDuration or MaxAnimePlayTime is nearly zero, not playing animation"), *(SourceActor->GetName()));
		return 0.0f;
	}

	return (AttackMontage->GetPlayLength() / PlayTime);
}

UAnimMontage* URPGGameplayAbilityBase::GetAnimMontage() const
{
	const ARPGAbilityActorBase* SourceActor = Cast<ARPGAbilityActorBase>(GetCurrentSourceObject());
	if (!SourceActor)
	{
		UE_LOG(LogAbilitySystem, Warning, TEXT("Ability %s GetAnimMontage SourceActor null"), *GetName());
		return nullptr;
	}

	return SourceActor->GetAbilityMontage();
}

float URPGGameplayAbilityBase::GetCooldownDuration() const
{
	const ARPGAbilityActorBase* SourceActor = Cast<ARPGAbilityActorBase>(GetCurrentSourceObject());

	if (!SourceActor)
	{
		UE_LOG(LogAbilitySystem, Warning, TEXT("Ability %s GetAnimationPlayRate SourceActor null"), *GetName());
		return 0.0f;
	}

	UE_LOG(LogAbilitySystem, Warning, TEXT("Ability %f cool down"), SourceActor->GetCooldownDuration());

	return SourceActor->GetCooldownDuration();
}

const FGameplayTagContainer* URPGGameplayAbilityBase::GetCooldownTags() const
{
	FGameplayTagContainer* MutableTags = const_cast<FGameplayTagContainer*>(&InternalUnionCooldownTags);
	MutableTags->Reset();

	const FGameplayTagContainer* ParentTags = Super::GetCooldownTags();

	if (ParentTags)
	{
		MutableTags->AppendTags(*ParentTags);
	}

	MutableTags->AppendTags(GetDynamicCooldownTags());

	if (MutableTags->Num() <= 0)
	{
		UE_LOG(LogAbilitySystem, Warning, TEXT("Ability %s GetCooldownTags Empty"), *GetName());
	}

	return MutableTags;
}

void URPGGameplayAbilityBase::ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	//Super::ApplyCooldown(Handle, ActorInfo, ActivationInfo);

	UGameplayEffect* CooldownGE = GetCooldownGameplayEffect();
	if (CooldownGE)
	{
		FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(CooldownGE->GetClass(), GetAbilityLevel());
		SpecHandle.Data.Get()->DynamicGrantedTags.AppendTags(GetDynamicCooldownTags());
		SpecHandle.Data.Get()->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.CooldownDuration")), GetCooldownDuration());
		ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
	}
}

bool URPGGameplayAbilityBase::K2_HasAuthority() const
{
	const FGameplayAbilityActorInfo* const CurrentActorInfoPtr = GetCurrentActorInfo();
	if (CurrentActorInfoPtr->OwnerActor.IsValid())
	{
		return CurrentActorInfoPtr->IsNetAuthority();
	}

	return false;
}

bool URPGGameplayAbilityBase::K2_IsLocalController() const
{
	return IsLocallyControlled();
}

bool URPGGameplayAbilityBase::CalculateDamageMagnitude(float& OutDamageMagnitude) const
{
	const ARPGAbilityActorBase* SourceActor = Cast<ARPGAbilityActorBase>(GetCurrentSourceObject());
	const ARPGCharacterBase* AvatarCharacter = Cast<ARPGCharacterBase>(GetAvatarActorFromActorInfo());

	if (!SourceActor || !AvatarCharacter)
	{
		UE_LOG(LogAbilitySystem, Warning, TEXT("Ability %s CalculateDamageMagnitude SourceActor or AvatarActor null"), *GetName());
		return false;
	}

	OutDamageMagnitude = SourceActor->GetBaseDamage();
	return true;
}

// Fill out your copyright notice in the Description page of Project Settings.ActivationOwnedTags


#include "Abilities/RPGActiveAbilityBase.h"
#include "Items/RPGMeleeWeaponActor.h"
#include "Character/RPGCharacterBase.h"
#include "Character/RPGInventoryComponent.h"
#include "Animation/AnimMontage.h"

URPGActiveAbilityBase::URPGActiveAbilityBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	auto ImplementedInBlueprint = [](const UFunction* Func) -> bool
	{
		return Func && ensure(Func->GetOuter())
			&& (Func->GetOuter()->IsA(UBlueprintGeneratedClass::StaticClass()) || Func->GetOuter()->IsA(UDynamicClass::StaticClass()));
	};

	{
		static FName FuncName = FName(TEXT("K2_CalculateDamage"));
		UFunction* ShouldRespondFunction = GetClass()->FindFunctionByName(FuncName);
		bHasBlueprintCalculateDamage = ImplementedInBlueprint(ShouldRespondFunction);
	}
	{
		static FName FuncName = FName(TEXT("K2_CalculateCooldownTime"));
		UFunction* CanActivateFunction = GetClass()->FindFunctionByName(FuncName);
		bHasBlueprintCalculateCooldownDuration = ImplementedInBlueprint(CanActivateFunction);
	}

	DamageGameplayEffectClass = nullptr;
	AnimationMontage = nullptr;
	OverrideAnimationPlayTime = -1.0f;

	bCooldownTagsInAbility = true;

	bCooldownSetByCaller = true;
	BaseCooldownDuration = 0.0f;

	bDamageSetByCaller = true;
	BaseDamage = 0.0f;
}

const FGameplayTagContainer* URPGActiveAbilityBase::GetCooldownTags() const
{
	FGameplayTagContainer* MutableTags = const_cast<FGameplayTagContainer*>(&InternalUnionCooldownTags);
	MutableTags->Reset();

	const FGameplayTagContainer* ParentTags = Super::GetCooldownTags();

	if (ParentTags)
	{
		MutableTags->AppendTags(*ParentTags);
	}

	//add the input cool down to the mutable tags
	MutableTags->AppendTags(GetAdditionalCooldownTags());

	if (MutableTags->Num() <= 0)
	{
		UE_LOG(LogAbilitySystem, Warning, TEXT("Ability %s GetCooldownTags Empty"), *GetName());
	}

	return MutableTags;
}

void URPGActiveAbilityBase::ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	//Super::ApplyCooldown(Handle, ActorInfo, ActivationInfo);

	UGameplayEffect* CooldownGE = GetCooldownGameplayEffect();
	if (CooldownGE)
	{
		FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(CooldownGE->GetClass(), GetAbilityLevel());
		SpecHandle.Data.Get()->DynamicGrantedTags.AppendTags(GetAdditionalCooldownTags());

		if (bCooldownSetByCaller)
		{
			SpecHandle.Data.Get()->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.CooldownDuration")), GetCooldownDuration());
		}
		
		ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
	}
}

float URPGActiveAbilityBase::GetCooldownDuration() const
{
	if (bHasBlueprintCalculateCooldownDuration)
	{
		return K2_CalculateCooldownDuration();
	}
	
	return BaseCooldownDuration;
}

float URPGActiveAbilityBase::GetBaseCooldownDuration() const
{
	return BaseCooldownDuration;
}

UAnimMontage* URPGActiveAbilityBase::GetAnimationMontage() const
{
	return AnimationMontage;
}

float URPGActiveAbilityBase::GetAnimationPlayRate() const
{
	if (!AnimationMontage)
	{
		UE_LOG(LogAbilitySystem, Warning, TEXT("URPGGameplayAbilityBase::GetAnimationPlayRate: Ability %s AnimationMontage null"), *GetName());
		return 0.0f;
	}

	//the play time cannot be more than the cool down, we wanna finish anim before we can attack next
	//we take the min from the cool down or attack speed
	const float CooldownDuration = GetCooldownDuration(); //for melee attacks, the cool down is also our attack speed

	//we take the min of the 2, we don't want to still be playing the animation and we don't to play an animation at a rate that looks very slow if the
	//cool down is really long
	const float DefaultPlayTime = (OverrideAnimationPlayTime > 0.0f) ? OverrideAnimationPlayTime : AnimationMontage->GetPlayLength();
	const float PlayTime = FMath::Min(CooldownDuration, DefaultPlayTime);

	if (FMath::IsNearlyZero(PlayTime))
	{
		UE_LOG(LogAbilitySystem, Warning, TEXT("URPGGameplayAbilityBase::GetAnimationPlayRate: Ability %s CooldownDuration or OverrideAnimationPlayTime is nearly zero, not playing animation"), *GetName());
		return 0.0f;
	}

	return (AnimationMontage->GetPlayLength() / PlayTime);
}

float URPGActiveAbilityBase::GetDamage() const
{
	if (bHasBlueprintCalculateDamage)
	{
		return K2_CalculateDamage();
	}

	return BaseDamage;
}

float URPGActiveAbilityBase::GetBaseDamage() const
{
	return BaseDamage;
}

FGameplayTagContainer URPGActiveAbilityBase::GetAdditionalCooldownTags() const
{
	const ARPGCharacterBase* AvatarCharacter = Cast<ARPGCharacterBase>(GetAvatarActorFromActorInfo());

	//an ai might not use the inventory system instead use pre-defined abilities, but this will still need to work on them too
	if (!AvatarCharacter)
	{
		//return empty
		return FGameplayTagContainer();
	}

	FGameplayTagContainer CooldownTags;

	//we need to get the cool down for the current equipped slot only if the controller is a player controller
	const URPGInventoryComponent* InventoryComponent = AvatarCharacter->GetInventoryComponent(); //ai would return null since the inventory component will not be set during OnPossessedBy
	if (GetActorInfo().PlayerController.IsValid() && InventoryComponent)
	{
		const FGameplayAbilitySpecHandle SpecHandle = GetCurrentAbilitySpecHandle();
		const ERPGAbilityInputID AbilityInput = InventoryComponent->GetAbilityHandleInputID(SpecHandle);
		CooldownTags.AppendTags(InventoryComponent->GetAbilityInputCooldownTag(AbilityInput)); //get the input slot cool down tags
	}

	if (bCooldownTagsInAbility)
	{
		CooldownTags.AppendTags(AbilityCooldownTags); //add the ability specific cool downs
	}

	if (CooldownTags.Num() == 0)
	{
		UE_LOG(LogAbilitySystem, Warning, TEXT("Ability %s URPGActiveAbilityBase::GetAdditionalCooldownTags empty, make sure ability or input slot has cooldown tags"), *GetName());
	}

	return CooldownTags;
}

void URPGActiveAbilityBase::ApplyDamageEffectToTargetData_Implementation(const FGameplayAbilityTargetDataHandle& TargetData) const
{
	//return if no target data or we are not authority, we are not predicting data since
	//Prediction keys are guaranteed to be valid during an atomic grouping of instructions "window" in GameplayAbilities starting with Activation from the activation prediction key. You can think of this as being only valid during one frame.
	//#TODO add Scoped Prediction Window to play anim montage etc..
	if (TargetData.Num() <= 0 || !GetActorInfo().OwnerActor->HasAuthority())
	{
		return;
	}

	//we will apply the damage effect even if the target has 100% damage immunity, the damage execution calculation will take care of damage block, amour etc..
	if (DamageGameplayEffectClass)
	{
		FGameplayEffectSpecHandle DamageSpecHandle = MakeOutgoingGameplayEffectSpec(DamageGameplayEffectClass, GetAbilityLevel());
		FGameplayEffectSpec* DamageSpec = DamageSpecHandle.Data.Get();

		//check if attack missed
		if (DamageSpec)
		{
			if (bDamageSetByCaller)
			{
				//#TODO calculate the damage with critical hit chance and damage %
				float DamageMagnitude = GetDamage();
				DamageSpec->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.Damage")), DamageMagnitude);
			}
	
			ApplyGameplayEffectSpecToTarget(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, DamageSpecHandle, TargetData);
		}
	}
	else
	{
		UE_LOG(LogAbilitySystem, Warning, TEXT("Ability %s DamageGameplayEffectClass null"), *GetName());
	}
}


bool URPGActiveAbilityBase::K2_HasAuthority() const
{
	const FGameplayAbilityActorInfo* const CurrentActorInfoPtr = GetCurrentActorInfo();
	if (CurrentActorInfoPtr->OwnerActor.IsValid())
	{
		return CurrentActorInfoPtr->IsNetAuthority();
	}

	return false;
}

bool URPGActiveAbilityBase::K2_IsLocalController() const
{
	return IsLocallyControlled();
}

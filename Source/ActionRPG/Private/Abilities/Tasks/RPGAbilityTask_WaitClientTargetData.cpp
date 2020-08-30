// Fill out your copyright notice in the Description page of Project Settings.

#include "Abilities/Tasks/RPGAbilityTask_WaitClientTargetData.h"
#include "AbilitySystemComponent.h"

URPGAbilityTask_WaitClientTargetData::URPGAbilityTask_WaitClientTargetData(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

URPGAbilityTask_WaitClientTargetData* URPGAbilityTask_WaitClientTargetData::WaitForClientTargetData(UGameplayAbility* OwningAbility, FName TaskInstanceName, bool TriggerOnce)
{
	URPGAbilityTask_WaitClientTargetData* MyObj = NewAbilityTask<URPGAbilityTask_WaitClientTargetData>(OwningAbility, TaskInstanceName);
	MyObj->bTriggerOnce = TriggerOnce;
	return MyObj;
}

void URPGAbilityTask_WaitClientTargetData::Activate()
{
	if (!Ability || !Ability->GetCurrentActorInfo()->IsNetAuthority())
	{
		return;
	}

	FGameplayAbilitySpecHandle	SpecHandle = GetAbilitySpecHandle();
	FPredictionKey ActivationPredictionKey = GetActivationPredictionKey();
	AbilitySystemComponent->AbilityTargetDataSetDelegate(SpecHandle, ActivationPredictionKey).AddUObject(this, &URPGAbilityTask_WaitClientTargetData::OnTargetDataReplicatedCallback);
}

void URPGAbilityTask_WaitClientTargetData::OnTargetDataReplicatedCallback(const FGameplayAbilityTargetDataHandle& Data, FGameplayTag ActivationTag)
{
	FGameplayAbilityTargetDataHandle MutableData = Data;
	AbilitySystemComponent->ConsumeClientReplicatedTargetData(GetAbilitySpecHandle(), GetActivationPredictionKey());

	if (ShouldBroadcastAbilityTaskDelegates())
	{
		ValidData.Broadcast(MutableData);
	}

	if (bTriggerOnce)
	{
		EndTask();
	}
}

void URPGAbilityTask_WaitClientTargetData::OnDestroy(bool AbilityEnded)
{
	if (AbilitySystemComponent)
	{
		FGameplayAbilitySpecHandle	SpecHandle = GetAbilitySpecHandle();
		FPredictionKey ActivationPredictionKey = GetActivationPredictionKey();
		AbilitySystemComponent->AbilityTargetDataSetDelegate(SpecHandle, ActivationPredictionKey).RemoveAll(this);
	}

	Super::OnDestroy(AbilityEnded);
}

// Fill out your copyright notice in the Description page of Project Settings.


#include "Abilities/RPGDamageExecutionCalculation.h"
#include "Character/RPGAttributeSetBase.h"
#include "AbilitySystemBlueprintLibrary.h"

struct FRPGDamageStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(Armor);

	// Meta attribute that we're passing into the ExecCalc via SetByCaller on the GESpec so we don't capture it.
	// We still need to declare and define it so that we can output to it.
	DECLARE_ATTRIBUTE_CAPTUREDEF(Damage);

	FRPGDamageStatics()
	{
		// Snapshot happens at time of GESpec creation

		// We're not capturing anything from the Source in this example, but there could be like AttackPower attributes that you might want.

		// Capture the Target's Armor. Don't snapshot (the false).
		DEFINE_ATTRIBUTE_CAPTUREDEF(URPGAttributeSetBase, Armor, Target, false);

		// The Target's received Damage. This is the value of health that will be subtracted on the Target. We're not capturing this.
		DEFINE_ATTRIBUTE_CAPTUREDEF(URPGAttributeSetBase, Damage, Target, false);
	}
};

static const FRPGDamageStatics& DamageStatics()
{
	static FRPGDamageStatics Statics;
	return Statics;
}

URPGDamageExecutionCalculation::URPGDamageExecutionCalculation()
{

}

void URPGDamageExecutionCalculation::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, OUT FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	UAbilitySystemComponent* TargetAbilitySystemComponent = ExecutionParams.GetTargetAbilitySystemComponent();
	UAbilitySystemComponent* SourceAbilitySystemComponent = ExecutionParams.GetSourceAbilitySystemComponent();

	AActor* SourceActor = SourceAbilitySystemComponent ? SourceAbilitySystemComponent->AvatarActor : nullptr;
	AActor* TargetActor = TargetAbilitySystemComponent ? TargetAbilitySystemComponent->AvatarActor : nullptr;

	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	FGameplayTagContainer AssetTags;
	Spec.GetAllAssetTags(AssetTags);

	// Gather the tags from the source and target as that can affect which buffs should be used
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;

	float Armor = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ArmorDef, EvaluationParameters, Armor);
	Armor = FMath::Max<float>(Armor, 0.0f);

	// SetByCaller Damage, damage should always be positive
	float UnmitigatedDamage = FMath::Max<float>(Spec.GetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.Damage"))), 0.0f);

	//send the game play event, Event.ReceiveHit to hit actor, this is the unmitigatedDamage, before armor or reflection etc.
	FGameplayTag EventTag(FGameplayTag::RequestGameplayTag(FName("Event.ReceiveHit"))); //#TODO send a different tag if attack missed? Event.ReceiveMissedHit
	FGameplayEventData Payload;
	Payload.Instigator = SourceActor;
	Payload.Target = TargetActor;
	Payload.EventMagnitude = UnmitigatedDamage;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(TargetActor, EventTag, Payload); //use the blueprint library

	//https://dota2.gamepedia.com/Armor
	//for now use https://leagueoflegends.fandom.com/wiki/Armor 100 / (100 + armor), easier calculation
	const float DamageMultiplier = CalculateArmorDamageMultiplier(Armor);
	const float MitigatedDamage = UnmitigatedDamage * DamageMultiplier;

	if (MitigatedDamage > 0.f)
	{
		// Set the Target's damage meta attribute
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(DamageStatics().DamageProperty, EGameplayModOp::Additive, MitigatedDamage));
	}
}

float URPGDamageExecutionCalculation::CalculateArmorDamageMultiplier(float ArmorValue) const
{
	if (ArmorValue >= 0.0f)
	{
		return (100 / (100 + ArmorValue));
	}
	else
	{
		return (2 - 100 / (100 - ArmorValue));
	}
}

/*
// Declare the attributes to capture and define how we want to capture them from the Source and Target.
struct GSDamageStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(Armor);

	// Meta attribute that we're passing into the ExecCalc via SetByCaller on the GESpec so we don't capture it.
	// We still need to declare and define it so that we can output to it.
	DECLARE_ATTRIBUTE_CAPTUREDEF(Damage);

	GSDamageStatics()
	{
		// Snapshot happens at time of GESpec creation

		// We're not capturing anything from the Source in this example, but there could be like AttackPower attributes that you might want.

		// Capture the Target's Armor. Don't snapshot (the false).
		DEFINE_ATTRIBUTE_CAPTUREDEF(UGSAttributeSetBase, Armor, Target, false);

		// The Target's received Damage. This is the value of health that will be subtracted on the Target. We're not capturing this.
		DEFINE_ATTRIBUTE_CAPTUREDEF(UGSAttributeSetBase, Damage, Target, false);
	}
};

static const GSDamageStatics& DamageStatics()
{
	static GSDamageStatics DStatics;
	return DStatics;
}

UGSDamageExecutionCalc::UGSDamageExecutionCalc()
{
	HeadShotMultiplier = 1.5f;

	RelevantAttributesToCapture.Add(DamageStatics().ArmorDef);

	// We don't include Damage here because we're not capturing it. It is generated inside the ExecCalc.
}

void UGSDamageExecutionCalc::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, OUT FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	UAbilitySystemComponent* TargetAbilitySystemComponent = ExecutionParams.GetTargetAbilitySystemComponent();
	UAbilitySystemComponent* SourceAbilitySystemComponent = ExecutionParams.GetSourceAbilitySystemComponent();

	AActor* SourceActor = SourceAbilitySystemComponent ? SourceAbilitySystemComponent->AvatarActor : nullptr;
	AActor* TargetActor = TargetAbilitySystemComponent ? TargetAbilitySystemComponent->AvatarActor : nullptr;

	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	FGameplayTagContainer AssetTags;
	Spec.GetAllAssetTags(AssetTags);

	// Gather the tags from the source and target as that can affect which buffs should be used
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;

	float Armor = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ArmorDef, EvaluationParameters, Armor);
	Armor = FMath::Max<float>(Armor, 0.0f);

	// SetByCaller Damage
	float Damage = FMath::Max<float>(Spec.GetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.Damage")), false, -1.0f), 0.0f);

	float UnmitigatedDamage = Damage; // Can multiply any damage boosters here

	// Check for headshot. There's only one character mesh here, but you could have a function on your Character class to return the head bone name
	const FHitResult* Hit = Spec.GetContext().GetHitResult();
	if (AssetTags.HasTagExact(FGameplayTag::RequestGameplayTag(FName("Effect.Damage.CanHeadShot"))) && Hit && Hit->BoneName == "b_head")
	{
		UnmitigatedDamage *= HeadShotMultiplier;
		FGameplayEffectSpec* MutableSpec = ExecutionParams.GetOwningSpecForPreExecuteMod();
		MutableSpec->DynamicAssetTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Effect.Damage.HeadShot")));
	}

	float MitigatedDamage = (UnmitigatedDamage) * (100 / (100 + Armor));

	if (MitigatedDamage > 0.f)
	{
		// Set the Target's damage meta attribute
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(DamageStatics().DamageProperty, EGameplayModOp::Additive, MitigatedDamage));
	}
}
*/

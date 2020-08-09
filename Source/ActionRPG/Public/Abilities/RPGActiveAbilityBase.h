// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "RPGActiveAbilityBase.generated.h"

/*
//enum only for the IsAuthority
UENUM(BlueprintType)
enum class EIsAuthority : uint8
{
	IA_HasAuthority,
	IA_Other
};*/

/**
 * Abstract GameplayAbilityBase, must be extended and implement the ability logic
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class ACTIONRPG_API URPGActiveAbilityBase : public UGameplayAbility
{
	GENERATED_BODY()

public:
	URPGActiveAbilityBase(const FObjectInitializer& ObjectInitializer);

	/** Returns all tags that are currently on cool down */
	virtual const FGameplayTagContainer* GetCooldownTags() const override;

	/** Applies CooldownGameplayEffect to the target */
	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;

	//Get the cool down duration modified by character stats, returns the base cool down duration if K2_GetCooldownDuration is not implemented in the child blueprint
	//DO NOT CALL THIS FROM K2_CalculateCooldownDuration
	UFUNCTION(BlueprintCallable, Category = "Gameplay Ability")
	virtual float GetCooldownDuration() const;

	//returns the base cool down
	UFUNCTION(BlueprintCallable, Category = "Gameplay Ability")
	float GetBaseCooldownDuration() const;

	UFUNCTION(BlueprintCallable, Category = "Gameplay Ability")
	class UAnimMontage* GetAnimationMontage() const;

	UFUNCTION(BlueprintCallable, Category = "Gameplay Ability")
	virtual float GetAnimationPlayRate() const;

	//Get the damage modified by other stats or whatever, returns the base damage if K2_CalculateDamage is not implemented in the child blueprint
	//DO NOT CALL THIS FROM K2_CalculateDamage
	UFUNCTION(BlueprintCallable, Category = "Gameplay Ability")
	virtual float GetDamage() const;

	//returns the base damage
	UFUNCTION(BlueprintCallable, Category = "Gameplay Ability")
	float GetBaseDamage() const;

	//blueprint helper function to check if the ability owner has authority
	UFUNCTION(BlueprintCallable, Category = "Gameplay Ability", DisplayName = "HasAuthority")
	bool K2_HasAuthority() const;

	//blueprint helper function to check if the ability owner is local controller
	UFUNCTION(BlueprintCallable, Category = "Gameplay Ability", DisplayName = "IsLocalController")
	bool K2_IsLocalController() const;
	
protected:
	/*the game play effect that is applied on attack, this will be the same since it doesn't change from 1 instance of the same ability to another*/
	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	TSubclassOf<UGameplayEffect> DamageGameplayEffectClass;

	/*most of our active abilities will have an animation*/
	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	class UAnimMontage* AnimationMontage;

	/*override the animation play time, must be greater than 0 to override or -1 to ignore this value
	 *the playtime of the animation will still be the min of the animation playtime and the cool down duration
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	float OverrideAnimationPlayTime;

	/*The ability tags, these will be added to the DynamicGrantedTags of the cool down effect, the ability must have a cool down class and should ideally have a SetByCaller cool down value
	 using the Data.CooldownDuration tag, abilities with the same cool down tags will share the cool down
	 you can also use a gameplay effect for the cool down, but using this will allow you to set a base cool down effect for all items in the game but change the tags*/
	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	FGameplayTagContainer AbilityCooldownTags;

	/*SetByCaller cool down value using the Data.CooldownDuration tag, can ignore this value if needed by not taking in SetByCaller tag of Data.CooldownDuration*/
	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	float BaseCooldownDuration;

	/*SetByCaller cool down value using the Data.CooldownDuration tag, can ignore this value if needed by not taking in SetByCaller tag of Data.CooldownDuration*/
	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	float BaseDamage;

	/**get the dynamic cool down tags of the input to which this ability is bound to and the AbilityCooldownTags*/
	FGameplayTagContainer GetAdditionalCooldownTags() const;

	//Handle target data, this is something common for all game play abilities, this will apply effects like freeze, light fire to target, reflect damage etc..
	//called by K2_ApplyDamageEffectToTargetData, just splitting the functionality from the blueprint node to make it more clear
	//will use GetDamage which returns the base damage unless K2_CalculateDamage is implemented in the child class
	void ApplyDamageEffectToTargetData(const FGameplayAbilityTargetDataHandle& TargetData) const;

	/*Blueprint node for HandleTargetData, base calls HandleTargetData. used to override the handle target data i.e. you want to fire effect on weapon or impact point*/
	UFUNCTION(BlueprintNativeEvent, BlueprintPure = false, Category = "Gameplay Ability", DisplayName = "ApplyDamageEffectToTargetData", meta = (ScriptName = "ApplyDamageEffectToTargetData"))
	void K2_ApplyDamageEffectToTargetData(const FGameplayAbilityTargetDataHandle& TargetData) const;

	UFUNCTION(BlueprintImplementableEvent, Category = Ability, DisplayName = "CalculateDamage", meta = (ScriptName = "CalculateDamage"))
	float K2_CalculateDamage() const;

	bool bHasBlueprintCalculateDamage;

	UFUNCTION(BlueprintImplementableEvent, Category = Ability, DisplayName = "CalculateCooldownDuration", meta = (ScriptName = "CalculateCooldownDuration"))
	float K2_CalculateCooldownDuration() const;

	bool bHasBlueprintCalculateCooldownDuration;

private:
	// Temp container that we will return the pointer to in GetCooldownTags().
	// This will be a union of our Dynamic CooldownTags and the cool down GE's cool down tags.
	UPROPERTY()
	FGameplayTagContainer InternalUnionCooldownTags;
};

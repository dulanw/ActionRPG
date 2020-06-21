// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "RPGGameplayAbilityBase.generated.h"

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
UCLASS(Abstract, BlueprintType)
class ACTIONRPG_API URPGGameplayAbilityBase : public UGameplayAbility
{
	GENERATED_BODY()

public:
	URPGGameplayAbilityBase(const FObjectInitializer& ObjectInitializer);
	
protected:
	//Handle target data, this is something common for all game play abilities, this will apply effects like freeze, light fire to target, reflect damage etc..
	//called by K2_handleTargetData, just splitting the functionality from the blueprint node to make it more clear
	void HandleTargetData(const FGameplayAbilityTargetDataHandle& TargetData) const;

	/*Blueprint node for HandleTargetData, base calls HandleTargetData. used to override the handle target data i.e. you want to fire effect on weapon or impact point*/
	UFUNCTION(BlueprintNativeEvent, BlueprintPure = false, Category = "Gameplay Ability", DisplayName = "HandleTargetData") // meta = (ScriptName = "HandleTargetData")
	void K2_HandleTargetData(const FGameplayAbilityTargetDataHandle& TargetData) const;

	/*the game play effect that is applied on attack, this will be the same since it doesn't change from 1 instance of the same ability to another*/
	UPROPERTY(EditAnywhere, Category = "Gameplay Effects")
	TSubclassOf<UGameplayEffect> DamageGameplayEffectClass;

	/*Calculate the damage magnitude, and return if damage effect should be applied (i.e if the user is blinded then 100% miss chance means all attacks will miss)
	 *return true if the event should be applied or false if the attack missed (or item could not be found/invalid data)
	 *the attack will cause all actors caught in the area to miss (if aoe attack), so it should be if the player is blinded by an effect
	 *otherwise individual actor misses due to dodge or evasion should be dealt with in DamageExecutionCalculation
	 *@param OutDamageMagnitude output the damage
	 */
	bool CalculateDamageMagnitude(float& OutDamageMagnitude) const;

	/**get the dynamic cool down tags that depends on the weapon and the slot, the slot cool down tags are specified in the character*/
	FGameplayTagContainer GetDynamicCooldownTags() const;

	UFUNCTION(BlueprintCallable)
	virtual float GetAnimPlayRate() const;

	UFUNCTION(BlueprintCallable)
	virtual class UAnimMontage* GetAnimMontage() const;

	virtual float GetCooldownDuration() const;

public:
	/** Returns all tags that are currently on cool down */
	virtual const FGameplayTagContainer* GetCooldownTags() const override;

	/** Applies CooldownGameplayEffect to the target */
	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;

	//blueprint helper function to check if the ability owner has authority
	UFUNCTION(BlueprintCallable, Category = "Gameplay Ability", DisplayName = "HasAuthority")
	bool K2_HasAuthority() const;

	//blueprint helper function to check if the ability owner is local controller
	UFUNCTION(BlueprintCallable, Category = "Gameplay Ability", DisplayName = "IsLocalController")
	bool K2_IsLocalController() const;

private:

	// Temp container that we will return the pointer to in GetCooldownTags().
	// This will be a union of our Dynamic CooldownTags and the cool down GE's cool down tags.
	UPROPERTY()
	FGameplayTagContainer InternalUnionCooldownTags;
};

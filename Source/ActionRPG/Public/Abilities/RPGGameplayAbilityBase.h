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
 *
 */
UCLASS(BlueprintType)
class ACTIONRPG_API URPGGameplayAbilityBase : public UGameplayAbility
{
	GENERATED_BODY()
	
protected:
	//Handle target data, this is something common for all game play abilities, this will apply effects like freeze, light fire to target, reflect damage etc..
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Gameplay Ability")
	void HandleTargetData(const FGameplayAbilityTargetDataHandle& TargetData) const;

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

	/**get the dynamic cool down tags that depends on the weapon and the slot*/
	FGameplayTagContainer GetDynamicCooldownTags() const;

	UFUNCTION(BlueprintCallable)
	virtual float GetAnimPlayRate() const;

	UFUNCTION(BlueprintCallable)
	virtual class UAnimMontage* GetAnimMontage() const;

	virtual float GetCooldownDuration() const;

public:
	/** Returns all tags that are currently on cooldown */
	virtual const FGameplayTagContainer* GetCooldownTags() const override;

	/** Applies CooldownGameplayEffect to the target */
	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;

	UFUNCTION(BlueprintCallable)
	bool IsAuthority() const;

	UFUNCTION(BlueprintCallable)
	bool IsLocalController() const;

private:
	// Temp container that we will return the pointer to in GetCooldownTags().
	// This will be a union of our Dynamic CooldownTags and the Cooldown GE's cooldown tags.
	UPROPERTY()
	FGameplayTagContainer InternalUnionCooldownTags;

};

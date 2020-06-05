// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "RPGAttributeSetBase.generated.h"

// Uses macros from AttributeSet.h
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
		GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
		GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
		GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
		GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * #TODO move to Abilities
 */
UCLASS()
class ACTIONRPG_API URPGAttributeSetBase : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	URPGAttributeSetBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// MaxHealth is its own attribute since GameplayEffects may modify it
	UPROPERTY(BlueprintReadOnly, Category = "Character Attribute Set | Health", ReplicatedUsing = OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(URPGAttributeSetBase, MaxHealth)

	UPROPERTY(EditDefaultsOnly, Category = "Character Attribute Set | Health", ReplicatedUsing = OnRep_Health)
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(URPGAttributeSetBase, Health)

	/*
	 * damage multiplier = 1 - ((0.052 × armor) ÷ (0.9 + 0.048 × |armor|))
	 * https://dota2.gamepedia.com/Armor
	 * No Max Armor since it will not change, items may modify it but it won't be reduced, maybe if there are armor reduction attacks.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Character Attribute Set | Armor", ReplicatedUsing = OnRep_Armor)
	FGameplayAttributeData Armor;
	ATTRIBUTE_ACCESSORS(URPGAttributeSetBase, Armor)

	/** 
	 * Damage is a meta attribute used by the DamageExecution to calculate final damage, which then turns into -Health
	 * Temporary value that only exists on the Server. Not replicated.
	 * Damage is a 'temporary' attribute used by the DamageExecution to calculate final damage, which then turns into -Health
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Character Attribute Set | Damage")
	FGameplayAttributeData Damage;
	ATTRIBUTE_ACCESSORS(URPGAttributeSetBase, Damage)

	//virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	/**
	* Called just before a GameplayEffect is executed to modify the base value of an attribute. No more changes can be made.
	* Note this is only called during an 'execute'. E.g., a modification to the 'base value' of an attribute. It is not called during an application of a GameplayEffect, such as a 5 ssecond +10 movement speed buff.
	*/
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth);

	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldHealth);

	UFUNCTION()
	void OnRep_Armor(const FGameplayAttributeData& OldArmor);

};

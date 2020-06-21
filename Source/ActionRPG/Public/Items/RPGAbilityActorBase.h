// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "RPGAbilityActorBase.generated.h"

//LogAbilityActor, used for logging ability actors unimplemented pure virtual functions
DECLARE_LOG_CATEGORY_EXTERN(LogAbilityActor, Log, All);

/**
 * Abstract class for pickup abilities inheriting from actor with visual representation in the level (can be dropped and picked up)
 * contains the base stats like damage, cool down tags, cool down duration, and animation
 */
UCLASS(abstract)
class ACTIONRPG_API ARPGAbilityActorBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARPGAbilityActorBase(const FObjectInitializer& ObjectInitializer);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//the cool down tag that will be added to the pawn in addition to the slot cool down
	UPROPERTY(EditAnywhere, Category = "Ability")
	FGameplayTagContainer CooldownTags;

	//the animation montage that should be played when the ability is activated, will always be completed by the cool down/fire rate is finished
	//play length won't be longer than the BaseAnimePlayTime
	UPROPERTY(EditAnywhere, Category = "Ability")
	class UAnimMontage* AbilityMontage;

	//how fast the sword should swing, the max amount
	//if the play time is less than the attack speed then the max animation time will be used otherwise it will be the attack speed/ cool down
	//since we want to complete the animation before the next time we activate it
	UPROPERTY(EditAnywhere, Category = "Ability|Stats")
	float BaseAnimePlayTime;

	/*the cool down duration for spells, also the fire rate for weapons. The animation time will always be less than or equal to this value*/
	UPROPERTY(EditAnywhere, Category = "Ability|Stats")
	float BaseCooldownDuration;

	/*the base damage, the item will only return the base damage, the ability will take into account the critical hits etc since some abilities might not have critical hits*/
	UPROPERTY(EditAnywhere, Category = "Ability|Stats")
	float BaseDamage;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//called when the instigator is passed to the client
	virtual void OnRep_Instigator() override;

	//Give the item to a new owner, set the owner and instigator
	UFUNCTION(BlueprintCallable, Category = "Ability")
	void GiveTo(class ARPGCharacterBase* NewOwner);

	//Get the ability granted, since this class is Abstract, the child class will need to define a member to hold the ability granted
	//do not call Super::GetAbilityGranted as the base class will crash
	UFUNCTION(BlueprintCallable, Category = "Ability")
	virtual TSubclassOf<class UGameplayAbility> GetAbilityGranted() const; //PURE_VIRTUAL(ARPGAbilityActorBase::GetAbilityGranted, return nullptr;) for pure virtual function

	virtual FGameplayTagContainer GetCooldownTags() const { return CooldownTags; }

	virtual class UAnimMontage* GetAbilityMontage() const { return AbilityMontage; }

	//the base animation play time, should not be changed by any stats as this is the only visual time
	virtual float GetBaseAnimPlayTime() const { return BaseAnimePlayTime; }

	//get the calculated cool down duration, this should take into any cool down reduction stats
	virtual float GetCooldownDuration() const { return BaseCooldownDuration; }

	//attack speed and damage should be scaled or critical hit chances applied on the ability and not from the weapon itself
	virtual float GetBaseDamage() const { return BaseDamage; }
};
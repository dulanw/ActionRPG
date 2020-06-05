// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "AttributeSet.h"
#include "RPGMeleeWeaponActor.generated.h"

UCLASS()
class ACTIONRPG_API ARPGMeleeWeaponActor : public AActor
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
	class USkeletalMeshComponent* Mesh;

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
	class UCapsuleComponent* CollisionComponent;

public:	
	// Sets default values for this actor's properties
	ARPGMeleeWeaponActor(const FObjectInitializer& ObjectInitializer);

	virtual void OnRep_Instigator() override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//TSet of ignored actors, can't use ignore on move since you would have to add it to both components
	TSet<AActor*> IgnoredActors;

	//called on blueprint when overlap begin
	//UFUNCTION(BlueprintImplementableEvent)
	//void Received_OnBeginOverlap(class UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//Give the item to a new owner, set the owner and instigator
	UFUNCTION(BlueprintCallable, Category = "Weapons")
	void GiveTo(class ARPGCharacterBase* NewOwner);

	//being the weapon attack, enable weapon capsule collision
	void BeginAttack();

	//end the weapon attack, disable weapon capsule collision (animation might not be finished yet)
	void EndAttack();

	UFUNCTION()
	void OnBeginOverlap(class UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	//#TODO move this to a struct
	//#TODO FScaleableFloat BaseDamage, damage before increment
	//Weapon/Item vars used in the game play ability 
	
	///#TODO move the properties below to parent class

protected:

	//the cool down tag that will be added to the pawn in addition to the slot cool down
	UPROPERTY(EditAnywhere, Category = "Weapon | Gameplay Ability")
	FGameplayTagContainer CooldownTags;

	UPROPERTY(EditAnywhere, Category = "Weapon | Gameplay Ability")
	class UAnimMontage* AbilityMontage;

	//how fast the sword should swing, the max amount
	//if the play time is less than the attack speed then the max anime time will be used otherwise it will be the attack speed
	UPROPERTY(EditAnywhere, Category = "Weapon | Stats")
	float BaseAnimePlayTime;



	/*the game play ability that is granted when the weapon is equipped*/	
	UPROPERTY(EditAnywhere, Category = "Weapon | Gameplay Ability")
	TSubclassOf<class UGameplayAbility> AbilityGranted; //#TODO make the ability granted specific to weapon type

	//the base attack speed, before attack speed increase perks are applied, time it takes to attack in seconds (1 = 1 second to attack)
	UPROPERTY(EditAnywhere, Category = "Weapon | Stats")
	float BaseAttackSpeed;

	//the base attack speed, before attack speed increase perks are applied
	UPROPERTY(EditAnywhere, Category = "Weapon | Stats")
	float BaseDamage;

public:
	//#TODO make a base class with Abstract specifier for the class and make these ones 

	virtual TSubclassOf<class UGameplayAbility> GetAbilityGranted() { return AbilityGranted; }

	virtual FGameplayTagContainer GetCooldownTags() const { return CooldownTags; }

	virtual class UAnimMontage* GetAbilityMontage() const { return AbilityMontage; }

	virtual float GetMaxAnimPlayTime() const { return BaseAnimePlayTime; }



	//these are specific for the melee weapon so they should not be in the 
	//attack speed and damage should be scaled or crit chances applied on the ability and not from the weapon itself
	float GetBaseAttackSpeed() const { return BaseAttackSpeed; }

	//attack speed and damage should be scaled or crit chances applied on the ability and not from the weapon itself
	float GetBaseDamage() const { return BaseDamage; }
};

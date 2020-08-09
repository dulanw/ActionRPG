// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/RPGActiveAbilityActor.h"
#include "RPGMeleeWeaponActor.generated.h"

UCLASS()
class ACTIONRPG_API ARPGMeleeWeaponActor : public ARPGActiveAbilityActor
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
	class UCapsuleComponent* CollisionComponent;

public:	
	// Sets default values for this actor's properties
	ARPGMeleeWeaponActor(const FObjectInitializer& ObjectInitializer);

	//bind delegate to the collision capsule
	virtual void PostInitializeComponents() override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//TSet of ignored actors, can't use ignore on move since you would have to add it to both components
	TSet<AActor*> IgnoredActors;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//being the weapon attack, enable weapon capsule collision
	void BeginAttack();

	//end the weapon attack, disable weapon capsule collision (animation might not be finished yet)
	void EndAttack();

	UFUNCTION()
	void OnBeginOverlap(class UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};

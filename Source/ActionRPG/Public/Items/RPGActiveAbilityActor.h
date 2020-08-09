// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/RPGInventoryItemBase.h"
#include "GameplayTagContainer.h"
#include "RPGActiveAbilityActor.generated.h"

//LogAbilityActor, used for logging ability actors unimplemented pure virtual functions
DECLARE_LOG_CATEGORY_EXTERN(LogAbilityActor, Log, All);

/**
 * Pickup able Inventory Item which grants an ability
 * contains the base stats like damage, cool down tags, cool down duration, and animation
 */
UCLASS(Blueprintable, BlueprintType)
class ACTIONRPG_API ARPGActiveAbilityActor : public ARPGInventoryItemBase
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARPGActiveAbilityActor(const FObjectInitializer& ObjectInitializer);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
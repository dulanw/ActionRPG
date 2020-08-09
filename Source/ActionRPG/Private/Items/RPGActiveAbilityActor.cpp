// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/RPGActiveAbilityActor.h"
#include "Character/RPGCharacterBase.h"

//LogAbilityActor, used for logging ability actors unimplemented pure virtual functions
DEFINE_LOG_CATEGORY(LogAbilityActor);

// Sets default values
ARPGActiveAbilityActor::ARPGActiveAbilityActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ItemType = ERPGItemType::ActiveItem;
}

// Called when the game starts or when spawned
void ARPGActiveAbilityActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ARPGActiveAbilityActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

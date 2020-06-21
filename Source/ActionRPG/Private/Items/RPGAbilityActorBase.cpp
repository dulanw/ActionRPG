// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/RPGAbilityActorBase.h"
#include "Character/RPGCharacterBase.h"

//LogAbilityActor, used for logging ability actors unimplemented pure virtual functions
DEFINE_LOG_CATEGORY(LogAbilityActor);

// Sets default values
ARPGAbilityActorBase::ARPGAbilityActorBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ARPGAbilityActorBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ARPGAbilityActorBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ARPGAbilityActorBase::OnRep_Instigator()
{
	Super::OnRep_Instigator();
}

void ARPGAbilityActorBase::GiveTo(ARPGCharacterBase* NewOwner)
{
	//can only pick up weapons on the server.
	if (!HasAuthority())
	{
		return;
	}

	SetInstigator(NewOwner);
	SetOwner(NewOwner);

	//PrimaryActorTick.AddPrerequisite(NewOwner, NewOwner->PrimaryActorTick);
	//ClientGiveTo(GetInstigator()); //not really needed since the instigator and the owner is replicated
	//implement the code below if there are issues with the owner replication
	/*
	void AFPSEquipableItem::ClientGiveTo_Implementation(APawn * NewInstigator)
	{
		if (NewInstigator != nullptr)
		{
			SetInstigator(NewInstigator);
			ClientGiveTo_Internal();
		}
	}

	void AFPSEquipableItem::ClientGiveTo_Internal()
	{
		checkSlow(Instigator != NULL);
		SetOwner(GetInstigator());
		FPSOwner = Cast<AFPSCharacterBase>(GetInstigator());
		checkSlow(FPSOwner != NULL);
		PrimaryActorTick.AddPrerequisite(FPSOwner, FPSOwner->PrimaryActorTick);
	}*/
}

TSubclassOf<UGameplayAbility> ARPGAbilityActorBase::GetAbilityGranted() const
{
	//print message, this will crash during runtime so will unimplemented, included both just incase
	UE_LOG(LogAbilitySystem, Fatal, TEXT("%s GetAbilityGranted pure virtual function not implemented"), *GetName());
	unimplemented();

	return nullptr;
}


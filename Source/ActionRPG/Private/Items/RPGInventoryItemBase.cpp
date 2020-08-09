// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/RPGInventoryItemBase.h"
#include "Character/RPGCharacterBase.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ARPGInventoryItemBase::ARPGInventoryItemBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SetReplicates(true);
	SetReplicateMovement(false);

	//need a empty scene component for the root since it will make the mesh the root otherwise
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	InteractiveCollision = CreateDefaultSubobject<USphereComponent>(TEXT("InteractiveCollision"));
	InteractiveCollision->SetupAttachment(RootComponent);
	InteractiveCollision->SetSphereRadius(50.0f);
	InteractiveCollision->SetCollisionProfileName(TEXT("Interactable"));
	InteractiveCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly); //start with physics enabled

	TurntableAttachOffset = CreateDefaultSubobject<USceneComponent>(TEXT("TurntableAttachOffset"));
	TurntableAttachOffset->SetupAttachment(RootComponent);

	EquippedAttachOffset = CreateDefaultSubobject<USceneComponent>(TEXT("EquippedAttachOffset"));
	EquippedAttachOffset->SetupAttachment(RootComponent);
	EquippedAttachOffset->SetRelativeLocationAndRotation(FVector::ZeroVector, FRotator::ZeroRotator);

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(TurntableAttachOffset); //start with attached to the turntable attach point and will reset to root when picked up or to a different attach point when needed 

	ItemType = ERPGItemType::None;
	bIsEquipped = false;
}

void ARPGInventoryItemBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//DOREPLIFETIME(ARPGInventoryItemBase, AvatarCharacter);
}

void ARPGInventoryItemBase::OnEnterInventory(class AActor* NewOwner, class ARPGCharacterBase* NewAvatarCharacter /*= nullptr*/)
{
	SetOwner(NewOwner);

	//we don't want to set the new avatar character if we are calling OnEnterInventory from OnRep_Owner where we can't be sure that the AvatarCharacter was replicated
	if (NewAvatarCharacter)
	{
		SetInstigator(NewAvatarCharacter);
		AvatarCharacter = NewAvatarCharacter;
	}

	//we need to check if this item was already equipped on the client as the current weapon maybe replicated earlier than the owner of this
	//that would call OnEnterInventory even after we called OnEquip
	if (!bIsEquipped)
	{
		DetachMeshFromPawn();
	}
}

void ARPGInventoryItemBase::OnLeaveInventory()
{
	SetOwner(nullptr);
	SetInstigator(nullptr);
	AvatarCharacter = nullptr;

	bIsEquipped = false;

	SetActorTickEnabled(true);
	Mesh->SetHiddenInGame(false);
	Mesh->AttachToComponent(TurntableAttachOffset, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	InteractiveCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void ARPGInventoryItemBase::OnEquip(class ARPGCharacterBase* NewAvatarCharacter, class AActor* NewOwner /*= nullptr*/)
{
	//set the owner here as well, since if the item is equipped straight away, then the owner might not have replicated yet
	if (NewOwner)
	{
		SetOwner(NewOwner);
	}

	SetInstigator(AvatarCharacter);
	AvatarCharacter = NewAvatarCharacter;

	bIsEquipped = true;
	AttachMeshToPawn();
	OnEquipFinished(NewAvatarCharacter);	//#TODO equip anim or particle effect
}

void ARPGInventoryItemBase::OnEquipFinished(ARPGCharacterBase* NewAvatarCharacter)
{

}

void ARPGInventoryItemBase::OnUnEquip()
{
	//if we drop/swap the item while it's equipped, then the client might have received the owner replication first before the unequip
	//so we don't want to hide it if it's visible on the ground
	if (bIsEquipped)
	{
		DetachMeshFromPawn();

		AvatarCharacter = nullptr;
		bIsEquipped = false;
	}
}

// Called when the game starts or when spawned
void ARPGInventoryItemBase::BeginPlay()
{
	Super::BeginPlay();
}

void ARPGInventoryItemBase::OnRep_Owner()
{
	Super::OnRep_Owner();

	if (GetOwner())
	{
		OnEnterInventory(GetOwner());
	}
	else
	{
		OnLeaveInventory();
	}
}

void ARPGInventoryItemBase::AttachMeshToPawn()
{
	if (AvatarCharacter)
	{
		DetachMeshFromPawn();

		USkeletalMeshComponent* SkMesh = AvatarCharacter->GetMesh();
		if (SkMesh)
		{
			AttachToComponent(SkMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("hand_r"));
		}

		Mesh->SetHiddenInGame(false);
		Mesh->AttachToComponent(EquippedAttachOffset, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	}
}

void ARPGInventoryItemBase::DetachMeshFromPawn()
{
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	SetActorLocation(FVector(0.0f)); //#TODO does it stop replicating if out of range??

	SetActorTickEnabled(false);
	Mesh->SetHiddenInGame(true);
	InteractiveCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

// Called every frame
void ARPGInventoryItemBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//if not attached to something then we assume that the item is dropped
	if (!GetOwner())
	{
		FRotator ActorRotation = GetActorRotation();
		ActorRotation.Yaw += 360.0f * DeltaTime;
		SetActorRotation(ActorRotation);
	}
}

/*
void ARPGInventoryItemBase::GiveTo(class AActor* NewOwner, class ACharacter* NewAvatarCharacter / *= nullptr* /)
{
	//SetInstigator(NewOwner); //SetInstigator needs a pawn but the owner might not be a pawn since it can be the playerstate
	SetOwner(NewOwner);

	//the newAvatarCharacter might be null, since it's possible to set only the NewOwner
	if (NewOwner && NewAvatarCharacter)
	{
		AvatarCharacter = NewAvatarCharacter;
	}
	else if (!NewOwner)
	{
		AvatarCharacter = nullptr;
	}

	/ *
	//can only pick up weapons on the server.
	if (!HasAuthority())
	{
		return;
	}
	* /

	/ *
	//PrimaryActorTick.AddPrerequisite(NewOwner, NewOwner->PrimaryActorTick);
	//ClientGiveTo(GetInstigator()); //not really needed since the instigator and the owner is replicated
	//implement the code below if there are issues with the owner replication
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
	}
	* /
}*/

//ARPGInventoryItemBase::GetAbilityGranted()
//{
//	//print message, this will crash during runtime so will unimplemented, included both just incase
//	UE_LOG(LogAbilitySystem, Fatal, TEXT("%s GetAbilityGranted pure virtual function not implemented"), *GetName());
//	unimplemented();
//
//	return nullptr;
//}

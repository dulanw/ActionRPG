// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/RPGMeleeWeaponActor.h"
#include "Character/RPGCharacterBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/PrimitiveComponent.h"
#include "AbilitySystemBlueprintLibrary.h"


// Sets default values
ARPGMeleeWeaponActor::ARPGMeleeWeaponActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);

	CollisionComponent = CreateDefaultSubobject<UCapsuleComponent>("Collision");
	CollisionComponent->SetupAttachment(RootComponent); //attach to root so that mesh scaling doesn't effect the collision, scale the whole actor if you need to do that
	CollisionComponent->SetCapsuleSize(15.0f, 40.0f, false);
	CollisionComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 40.0f));
	CollisionComponent->SetCollisionProfileName(TEXT("Melee"));
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision); //disable collision by default
	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &ARPGMeleeWeaponActor::OnBeginOverlap);

	BaseAttackSpeed = 1.0f;
	BaseDamage = 10.0f;
}

void ARPGMeleeWeaponActor::OnRep_Instigator()
{
	Super::OnRep_Instigator();
}

// Called when the game starts or when spawned
void ARPGMeleeWeaponActor::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ARPGMeleeWeaponActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ARPGMeleeWeaponActor::GiveTo(ARPGCharacterBase* NewOwner)
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


void ARPGMeleeWeaponActor::BeginAttack()
{
	IgnoredActors.Empty();

	//hit detection done on the server but predicted on the client
	bool bEnableCollision = GetInstigator()->IsLocallyControlled() || GetOwner()->HasAuthority();

	ECollisionEnabled::Type CollisionEnabled = bEnableCollision ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision;
	CollisionComponent->SetCollisionEnabled(CollisionEnabled);	//query only since that enables overlaps, doesn't need physics
}

void ARPGMeleeWeaponActor::EndAttack()
{
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ARPGMeleeWeaponActor::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//even though we ignore our actor, our actor does not 
	if (OtherActor == GetOwner() || IgnoredActors.Contains(OtherActor)) 
	{
		return;
	}

	IgnoredActors.Add(OtherActor); //ignore the actor so we won't hit them twice in the same attack
	
	FGameplayTag EventTag(FGameplayTag::RequestGameplayTag(FName("Event.Hit.Melee"))); //#TODO weapon actor specific tags?
	FGameplayEventData Payload;
	Payload.Instigator = this;
	Payload.Target = OtherActor;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetOwner(), EventTag, Payload); //use the blueprint library
}
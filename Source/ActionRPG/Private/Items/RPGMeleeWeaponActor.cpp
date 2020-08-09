// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/RPGMeleeWeaponActor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/PrimitiveComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Character/RPGCharacterBase.h"


// Sets default values
ARPGMeleeWeaponActor::ARPGMeleeWeaponActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	CollisionComponent = CreateDefaultSubobject<UCapsuleComponent>("Collision");
	CollisionComponent->SetupAttachment(GetMesh()); //attach to root so that mesh scaling doesn't effect the collision, scale the whole actor if you need to do that
	CollisionComponent->SetCapsuleSize(15.0f, 40.0f, false);
	CollisionComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 40.0f));
	CollisionComponent->SetCollisionProfileName(TEXT("OverlapOnlyPawn"));
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision); //disable collision by default
}

void ARPGMeleeWeaponActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	//just make sure we don't get any problems with the blueprints
	//this will crash if you had in the constructor and moved it, will need to reparent the blueprint classes
	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &ARPGMeleeWeaponActor::OnBeginOverlap);
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


void ARPGMeleeWeaponActor::BeginAttack()
{
	IgnoredActors.Empty();

	ECollisionEnabled::Type CollisionEnabled = GetOwner()->HasAuthority() ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision;
	CollisionComponent->SetCollisionEnabled(CollisionEnabled);	//query only since that enables overlaps, doesn't need physics
}

void ARPGMeleeWeaponActor::EndAttack()
{
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ARPGMeleeWeaponActor::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//even though we ignore our actor, our actor does not 
	if (AvatarCharacter == OtherActor || IgnoredActors.Contains(OtherActor))
	{
		return;
	}

	IgnoredActors.Add(OtherActor); //ignore the actor so we won't hit them twice in the same attack
	
	FGameplayTag EventTag(FGameplayTag::RequestGameplayTag(FName("Event.Hit.Melee"))); //#TODO weapon actor specific tags?
	FGameplayEventData Payload;
	Payload.Instigator = this;
	Payload.Target = OtherActor;

	//we assume that the player's ability system component is on the AvatarCharacter
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(AvatarCharacter, EventTag, Payload); //use the blueprint library
}
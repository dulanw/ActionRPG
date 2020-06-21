// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/RPGMeleeWeaponActor.h"
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
}

TSubclassOf<UGameplayAbility> ARPGMeleeWeaponActor::GetAbilityGranted() const
{
	return AbilityGranted;
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
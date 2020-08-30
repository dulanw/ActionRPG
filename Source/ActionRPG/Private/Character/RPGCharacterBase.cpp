// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "Character/RPGCharacterBase.h"
#include "Character/RPGAttributeSetBase.h"
#include "Player/RPGPlayerState.h"
#include "Player/RPGPlayerController.h"
#include "Character/RPGInventoryComponent.h"
#include "Items/RPGInventoryItemBase.h"
#include "Items/RPGMeleeWeaponActor.h"
#include "UI/RPGInventoryUI.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Net/UnrealNetwork.h"
#include "ActionRPG.h"

#include "DrawDebugHelpers.h"

FName ARPGCharacterBase::AbilitySystemComponentName(TEXT("AbilitySystemComponent"));

//use the character attribute set name to override the attribute set on child classes
FName ARPGCharacterBase::CharacterAttributeSetName(TEXT("CharacterAttributeSet")); 

//////////////////////////////////////////////////////////////////////////
// AActionRPGCharacter

ARPGCharacterBase::ARPGCharacterBase(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	: Super(ObjectInitializer)
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.75f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->TargetOffset = FVector(0.0f, 0.0f, 100.0f); //move it slightly up
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and animation blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(ARPGCharacterBase::AbilitySystemComponentName);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	AbilitySystemComponent->SetIsReplicated(true);

	CharacterAttributeSet = CreateDefaultSubobject<URPGAttributeSetBase>(ARPGCharacterBase::CharacterAttributeSetName);

	InteractTraceDistance = 1000.0f;
	InteractDistance = 200.0f;

	ItemToPickup = nullptr;
	bInventoryUIOpen = false;
}

void ARPGCharacterBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->OnAbilityEnded.AddUObject(this, &ARPGCharacterBase::OnAbilityEnd);
	}
}

void ARPGCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void ARPGCharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	//if we have a valid player state then we get the inventory, it should only have a player state if we have a valid controller but we still need to check if it's local
	ARPGPlayerState* PS = GetPlayerState<ARPGPlayerState>();
	if (PS)
	{
		InventoryComponent = PS->GetPlayerInventoryComponent();

		ARPGPlayerController* PC = GetController<ARPGPlayerController>();
		if (PC && PC->IsLocalController())
		{
			//if we are the local player then we need to do the HUD and then the rest of the stuff
			//just check for PC since player controller will not be valid on simulated proxies

			InventoryUI = CreateInventoryUIWidget(PC, InventoryComponent);
		}
	}

	//we need to InitAbilityActorInfo for both the player and ai
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}
}

void ARPGCharacterBase::OnRep_Controller()
{
	Super::OnRep_Controller();

/*
	ARPGPlayerController* PC = GetController<ARPGPlayerController>();
	if (PC)
	{
		PC->GetPlayerState()
	}*/
}

void ARPGCharacterBase::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	//We set the inventory component from the player state on all clients, including simulated proxy
	ARPGPlayerState* PS = GetPlayerState<ARPGPlayerState>();
	if (PS)
	{
		//if we have a valid player state then we get the inventory
		InventoryComponent = PS->GetPlayerInventoryComponent();

		ARPGPlayerController* PC = GetController<ARPGPlayerController>();
		if (PC) //this is the same as PossessedBy but we don't have to check if it's a local player controller since only the owners controller is replicated
		{
			//if we are the local player then we need to do the HUD and then the rest of the stuff
			//just check for PC since player controller will not be valid on simulated proxies

			InventoryUI = CreateInventoryUIWidget(PC, InventoryComponent);

			//don't need to init the ability on simulated proxies (or ai, since for ai it will be done on the PossessedBy)
			if (AbilitySystemComponent)
			{
				AbilitySystemComponent->InitAbilityActorInfo(this, this);
			}
		}
	}
}

URPGInventoryUI* ARPGCharacterBase::CreateInventoryUIWidget(APlayerController* LocalController, URPGInventoryComponent* InInventoryComponent)
{
	URPGInventoryUI* Widget = CreateWidget<URPGInventoryUI>(LocalController, InventoryUIClass);
	Widget->OnInventorySelectSlot.AddUObject(this, &ARPGCharacterBase::OnSelectSlotInventoryUI);
	Widget->OnInventoryClose.AddUObject(this, &ARPGCharacterBase::OnCloseInventoryUI);
	Widget->InitializeWidget(InInventoryComponent);

	return Widget;
}

void ARPGCharacterBase::OnSelectSlotInventoryUI(ERPGInventorySlot SelectedSlot)
{
	if (InventoryComponent)
	{
		Server_PickupItem(ItemToPickup, SelectedSlot); //calling Server RPC on listen-server, doesn't really matter since it runs on it as well
	}

	ItemToPickup = nullptr;
	bInventoryUIOpen = false;
	if (InventoryUI)
	{
		InventoryUI->Hide();
	}
}

void ARPGCharacterBase::OnCloseInventoryUI()
{
	ItemToPickup = nullptr;
	bInventoryUIOpen = false;
	if (InventoryUI)
	{
		InventoryUI->Hide();
	}
}

void ARPGCharacterBase::Interact()
{
	const APlayerController* PC = GetController<APlayerController>();

	//only if we are player and local controller
	if (!PC || !PC->IsLocalController())
	{
		UE_LOG(LogTemp, Warning, TEXT("Must be called on Local Controller"));
		return;
	}

	FVector ViewLocation;
	FRotator ViewRotation;
	PC->GetPlayerViewPoint(ViewLocation, ViewRotation);

	FVector EndLocation = ViewLocation + ViewRotation.Vector() * InteractTraceDistance;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(ARPGCharacterBase), false, this); //ignore the character

	FHitResult OutHit(1.0f);
	GetWorld()->LineTraceSingleByChannel(OutHit, ViewLocation, EndLocation, COLLISION_INTERACT, Params);

	DrawDebugLine(GetWorld(), ViewLocation, EndLocation, FColor::Red, true);

	if (OutHit.IsValidBlockingHit())
	{
		ARPGInventoryItemBase* InventoryItem = Cast<ARPGInventoryItemBase>(OutHit.GetActor());
		if (InventoryItem && InventoryItem->ItemType > ERPGItemType::PassiveItem && InventoryUI) //we should only open the inventory screen if it's an active item or weapon
		{
			ItemToPickup = InventoryItem;
			bInventoryUIOpen = true;
			InventoryUI->Show(InventoryItem);
		}
	}
}

void ARPGCharacterBase::Server_PickupItem_Implementation(ARPGInventoryItemBase* Item, ERPGInventorySlot Slot /*= ERPGInventorySlot::None*/)
{
	if (!InventoryComponent)
	{
		return;
	}

	if (Item && Item->ItemType > ERPGItemType::PassiveItem && Slot != ERPGInventorySlot::None)
	{ 
		InventoryComponent->AddItem(Item, Slot);
	}
}

bool ARPGCharacterBase::Server_PickupItem_Validate(ARPGInventoryItemBase* Item, ERPGInventorySlot Slot /*= ERPGInventorySlot::None*/)
{
	return true;
}

UAbilitySystemComponent* ARPGCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

URPGAttributeSetBase* ARPGCharacterBase::GetCharacterAttributeSet()
{
	return CharacterAttributeSet;
}

void ARPGCharacterBase::OnAbilityEnd(const FAbilityEndedData& AbilityEndData)
{
	OnAbilityEnded.Broadcast(AbilityEndData.AbilityThatEnded, AbilityEndData.AbilitySpecHandle, AbilityEndData.bReplicateEndAbility, AbilityEndData.bWasCancelled);
}

bool ARPGCharacterBase::ActivateAbilitiesWithInputID(ERPGAbilityInputID InputID, bool bAllowRemoteActivation /*= true*/)
{
	if (GetInventoryComponent())
	{
		return GetInventoryComponent()->ActivateAbilitiesWithInputID(InputID, bAllowRemoteActivation);
	}

	return false;
}

ARPGInventoryItemBase* ARPGCharacterBase::GetCurrentWeapon() const
{
	if (GetInventoryComponent())
	{
		return GetInventoryComponent()->GetCurrentWeapon();
	}

	return nullptr;
}

void ARPGCharacterBase::OnMeleeAttackStarted_Implementation()
{
	ARPGInventoryItemBase* CurrentWeapon = GetCurrentWeapon();
	ARPGMeleeWeaponActor* MeleeWeapon = (CurrentWeapon) ? Cast<ARPGMeleeWeaponActor>(CurrentWeapon) : nullptr;
	if (MeleeWeapon)
	{
		MeleeWeapon->BeginAttack();
	}
}

void ARPGCharacterBase::OnMeleeAttackEnded_Implementation()
{
	ARPGInventoryItemBase* CurrentWeapon = GetCurrentWeapon();
	ARPGMeleeWeaponActor* MeleeWeapon = (CurrentWeapon) ? Cast<ARPGMeleeWeaponActor>(CurrentWeapon) : nullptr;
	if (MeleeWeapon)
	{
		MeleeWeapon->EndAttack();
	}
}

//////////////////////////////////////////////////////////////////////////
// Input
void ARPGCharacterBase::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up game play key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &ARPGCharacterBase::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ARPGCharacterBase::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turn rate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ARPGCharacterBase::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ARPGCharacterBase::LookUpAtRate);

	//#TODO input for charging the attack, call normal attack on IE_Released
	//if the weapon can be charged then attack on IE_Released, otherwise attack on IE_Pressed
	//Bind the input ability, use the ability that is taken from the weapon
	PlayerInputComponent->BindAction("NormalAttack", IE_Pressed, this, &ARPGCharacterBase::NormalAttack);

	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &ARPGCharacterBase::Interact);
}

void ARPGCharacterBase::NormalAttack()
{
	//#TODO FRPGInventorySlot CurrentWeaponSlot, change this instead of having a pointer to an actor
	ActivateAbilitiesWithInputID(ERPGAbilityInputID::PrimaryFire);
}

void ARPGCharacterBase::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ARPGCharacterBase::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ARPGCharacterBase::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ARPGCharacterBase::MoveRight(float Value)
{
	if ( (Controller != NULL) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

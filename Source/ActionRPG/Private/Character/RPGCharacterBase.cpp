// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "Character/RPGCharacterBase.h"
#include "Character/RPGAttributeSetBase.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Items/RPGMeleeWeaponActor.h"
#include "Net/UnrealNetwork.h"

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

	//default SlotCooldownTags init
	SlotCooldownTags.Add(ERPGInventorySlot::WeaponSlot1, FGameplayTagContainer(FGameplayTag::RequestGameplayTag(FName("Cooldown.Slot.WeaponSlot1"))));
	SlotCooldownTags.Add(ERPGInventorySlot::WeaponSlot2, FGameplayTagContainer(FGameplayTag::RequestGameplayTag(FName("Cooldown.Slot.WeaponSlot2"))));
	SlotCooldownTags.Add(ERPGInventorySlot::AbilitySlot1, FGameplayTagContainer(FGameplayTag::RequestGameplayTag(FName("Cooldown.Slot.AbilitySlot1"))));
	SlotCooldownTags.Add(ERPGInventorySlot::AbilitySlot2, FGameplayTagContainer(FGameplayTag::RequestGameplayTag(FName("Cooldown.Slot.AbilitySlot2"))));
}

void ARPGCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ARPGCharacterBase, SlottedInventory, COND_OwnerOnly);
}

void ARPGCharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
		AbilitySystemComponent->RefreshAbilityActorInfo();
	}

	SetOwner(NewController);
}

UAbilitySystemComponent* ARPGCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

URPGAttributeSetBase* ARPGCharacterBase::GetCharacterAttributeSet()
{
	return CharacterAttributeSet;
}

ERPGInventorySlot ARPGCharacterBase::GetGameplayAbilityHandleSlot(const FGameplayAbilitySpecHandle& AbilitySpecHandle) const
{
	const FRPGInventorySlotData* const FoundSlotData = SlottedInventory.FindByKey(AbilitySpecHandle);

	if (FoundSlotData)
	{
		return FoundSlotData->Slot;
	}

	return ERPGInventorySlot::SLOT_NONE;
}

FGameplayTagContainer ARPGCharacterBase::GetSlotCooldownTags(const ERPGInventorySlot& Slot) const
{
	const FGameplayTagContainer* CooldownTag = SlotCooldownTags.Find(Slot);

	if (CooldownTag)
	{
		return *CooldownTag;
	}

	return FGameplayTagContainer();
}

bool ARPGCharacterBase::ActivateAbilitiesWithItemSlot(ERPGInventorySlot Slot, bool bAllowRemoteActivation /*= true*/)
{
	//Find returns a pointer, so make sure to dereference it
	FRPGInventorySlotData* FoundSlotData = SlottedInventory.FindByKey(Slot);

	//make sure the slot data is found and the ability spec handle is valid
	if (FoundSlotData && FoundSlotData->AbilitySpecHandle.IsValid() && AbilitySystemComponent)
	{
		return AbilitySystemComponent->TryActivateAbility(FoundSlotData->AbilitySpecHandle, bAllowRemoteActivation);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ARPGCharacterBase::ActivateAbilitiesWithItemSlot: ability component/handle NOT found, Make sure item/ability was added to slot"));
	}

	return false;
}

void ARPGCharacterBase::AddAbilityToSlot(ERPGInventorySlot Slot, TSubclassOf<UGameplayAbility> AbilityToAcquire, AActor* Actor /*= nullptr*/)
{
	//can't do anything without the ability system or it's not authority or we don't have an ability
	if (!AbilitySystemComponent || !HasAuthority() || !AbilityToAcquire)
	{
		return;
	}

	FRPGInventorySlotData* FoundSlotData = SlottedInventory.FindByKey(Slot);

	if (FoundSlotData)
	{
		//#TODO drop actor if found or destroy it....

		//remove the ability previous ability
		AbilitySystemComponent->ClearAbility(FoundSlotData->AbilitySpecHandle);

		//remove the index, just much easier than modifying the data depending on if we found it or not
		SlottedInventory.Remove(*FoundSlotData);
	}


	FGameplayAbilitySpecHandle NewAbilitySpecHandle = AcquireAbility(AbilityToAcquire, Actor);
	if (NewAbilitySpecHandle.IsValid())
	{
		SlottedInventory.Add(FRPGInventorySlotData(Slot, NewAbilitySpecHandle, Actor));
	}
}

FGameplayAbilitySpecHandle ARPGCharacterBase::AcquireAbility(TSubclassOf<UGameplayAbility> AbilityToAcquire, AActor* SourceObject /*= nullptr*/)
{
	/*just a fail safe, this will be getting called by AddItemToSlot and it should already check for the component and authority, so should not fail*/
	if (!AbilitySystemComponent || !HasAuthority() || !AbilityToAcquire)
	{
		return FGameplayAbilitySpecHandle();
	}

	//pass INDEX_NONE for InputID since we aren't using the input mapping
	return AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(AbilityToAcquire, 1, INDEX_NONE, SourceObject));
}

AActor* ARPGCharacterBase::GetCurrentWeapon() const
{
	//#TODO current weapon slot
	const FRPGInventorySlotData* FoundSlotData = SlottedInventory.FindByKey(ERPGInventorySlot::WeaponSlot1);

	if (FoundSlotData)
	{
		return FoundSlotData->Actor;
	}

	return nullptr;
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

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &ARPGCharacterBase::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &ARPGCharacterBase::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &ARPGCharacterBase::OnResetVR);

	//#TODO input for charging the attack, call normal attack on IE_Released
	//if the weapon can be charged then attack on IE_Released, otherwise attack on IE_Pressed
	//Bind the input ability, use the ability that is taken from the weapon
	PlayerInputComponent->BindAction("NormalAttack", IE_Pressed, this, &ARPGCharacterBase::NormalAttack);
}


void ARPGCharacterBase::OnRep_SlottedInventory()
{
	//#TODO call swap weapons or get the best next weapon, since the inventory might have dropped or swapped our current weapon
}

void ARPGCharacterBase::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void ARPGCharacterBase::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
		Jump();
}

void ARPGCharacterBase::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
		StopJumping();
}

void ARPGCharacterBase::NormalAttack()
{
	//#TODO FRPGInventorySlot CurrentWeaponSlot, change this instead of having a pointer to an actor
	ActivateAbilitiesWithItemSlot(ERPGInventorySlot::WeaponSlot1);
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

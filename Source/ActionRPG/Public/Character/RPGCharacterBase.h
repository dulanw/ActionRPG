// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "RPGCharacterBase.generated.h"


//#TODO Move CharacterBase->GetAbilitySystemComponent()->InitAbilityActorInfo(CharacterBase, CharacterBase); from controller to pawn OnRep_Controller

//enum used to bind the input to ability
//don't use since AI can't use it
UENUM(BlueprintType)
enum class ERPGInventorySlot : uint8
{
	SLOT_NONE		UMETA(DisplayName = "SLOT_NONE"),
	WeaponSlot1		UMETA(DisplayName = "Weapon Slot 1"),
	WeaponSlot2		UMETA(DisplayName = "Weapon Slot 2"),
	AbilitySlot1	UMETA(DisplayName = "Ability Slot 1"),
	AbilitySlot2	UMETA(DisplayName = "Ability Slot 2")
};

USTRUCT(BlueprintType)
struct FRPGInventorySlotData
{
	GENERATED_BODY()

	//the current slot
	UPROPERTY()
	ERPGInventorySlot Slot;

	//the current ability handle
	UPROPERTY()
	FGameplayAbilitySpecHandle AbilitySpecHandle;

	//Current actor, when picked up need to be assigned alongside the ability spec handle and removed when dropped
	//some might not have an actor but most should, since you are able to pick up and drop items (don't delete and respawn, just hide it)
	UPROPERTY()
	AActor* Actor;

	FRPGInventorySlotData()
		: Slot(ERPGInventorySlot::SLOT_NONE), AbilitySpecHandle(FGameplayAbilitySpecHandle()), Actor(nullptr) 
	{ 

	}

	FRPGInventorySlotData(ERPGInventorySlot InSlot, FGameplayAbilitySpecHandle InAbilitySpecHandle, AActor* InActor)
		: Slot(InSlot), AbilitySpecHandle(InAbilitySpecHandle), Actor(InActor)
	{

	}

	bool operator==(const FRPGInventorySlotData& Other) const
	{
		return (Slot == Other.Slot);
	}

	bool operator==(const ERPGInventorySlot& InSlot) const
	{
		return (InSlot == Slot);
	}

	bool operator==(const FGameplayAbilitySpecHandle& InAbilitySpecHandle) const
	{
		return (InAbilitySpecHandle == AbilitySpecHandle);
	}
};

UCLASS(config=Game)
class ARPGCharacterBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	/** Our ability system */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Abilities, meta = (AllowPrivateAccess = "true"))
	UAbilitySystemComponent* AbilitySystemComponent;

	//the cool down tag corresponding to each slot, if the tag or slot is empty from this list then there will be no cool down for that slot
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Abilities, meta = (AllowPrivateAccess = "true"))
	TMap<ERPGInventorySlot, FGameplayTagContainer> SlotCooldownTags;

	UPROPERTY()
	class URPGAttributeSetBase* CharacterAttributeSet;

public:
	ARPGCharacterBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PossessedBy(AController* NewController) override;

	//IAbilitySystemInterface function
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UFUNCTION(BlueprintCallable)
	virtual URPGAttributeSetBase* GetCharacterAttributeSet();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	static FName AbilitySystemComponentName;

	static FName CharacterAttributeSetName;

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

protected:

	/** Resets HMD orientation in VR. */
	void OnResetVR();

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

	/*input for normal attack*/
	void NormalAttack();

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

	/** Map of slot to ability granted by that slot. Only contain the currently usable abilities, so if the player has 2 weapons
	 * Only the currently equipped items ability will be in this, will need to change/update the SlottedAbilities when switching weapons*/
	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities | Inventory") //#TODO replicate to owner only
	//TMap<FRPGInventorySlots, FGameplayAbilitySpecHandle> SlottedAbilities;

	/*contains the abilities for the inventory slots, i.e only usable abilities and weapons, primary and secondary weapon included at the same time
	 *use a variable i.e. CurrentWeaponSlot = weaponslot1 etc.. 
	 *using a TArray instead of TMap since this needs to be replicated, use .Find with FRPGInventorySlot to find the corresponding ability and actor or 
	 *use the ability handle to find which slot it's occupying, which then you can use to find the cool down tag on SlotCooldownTags etc
	 */
	UPROPERTY(ReplicatedUsing = OnRep_SlottedInventory, VisibleAnywhere, BlueprintReadOnly, Category = "Abilities | Inventory")
	TArray<FRPGInventorySlotData> SlottedInventory;

	/** Called on owning client when SlottedInventory is replicated.*/
	UFUNCTION()
	virtual void OnRep_SlottedInventory();

public:
	//get the slotted inventory list
	TArray<FRPGInventorySlotData> GetSlottedInventory() { return SlottedInventory; }

	//Get the slot which the gameplay ability is slotted to. used to find the cooldown tag for the slot
	ERPGInventorySlot GetGameplayAbilityHandleSlot(const FGameplayAbilitySpecHandle& AbilitySpecHandle) const;

	FGameplayTagContainer GetSlotCooldownTags(const ERPGInventorySlot& Slot) const;

	/**
	 * Attempts to activate any ability in the specified item slot. Will return false if no activatable ability found or activation fails
	 * Returns true if it thinks it activated, but it may return false positives due to failure later in activation.
	 * If bAllowRemoteActivation is true, it will remotely activate local/server abilities, if false it will only try to locally activate the ability
	 */
	UFUNCTION(BlueprintCallable, Category = "Abilities")
	bool ActivateAbilitiesWithItemSlot(ERPGInventorySlot Slot, bool bAllowRemoteActivation = true);

	/*add an item to a slot, you should remove an item before adding, otherwise the actor might be lost if not dropped before
	 *the actor might be null since you might be given an ability without
	 *#TODO make it only take in an SlotItemActor or something similar
	 */
	UFUNCTION(BlueprintCallable, Category = "Abilities")
	void AddItemToSlot(ERPGInventorySlot Slot, TSubclassOf<UGameplayAbility> AbilityToAcquire, AActor* Actor = nullptr);

	/*Add an ability to the user with sourceObject
	 *returns the handle, or handle with INDEX_NONE if invalid GameplayAbility
	 *#TODO pass in level, and other values etc...
	 */
	FGameplayAbilitySpecHandle AcquireAbility(TSubclassOf<UGameplayAbility> AbilityToAcquire, AActor* SourceObject = nullptr);

	AActor* GetCurrentWeapon() const;
};


// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayAbilitySpec.h"
#include "Character/RPGInventoryComponent.h"
#include "RPGCharacterBase.generated.h"

/*Wrapper around OnAbilityEnded since it's not a dynamic delegate, cannot access FAbilityEndedData from blueprint so pass the individual data*/
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FRPGGameplayAbilityEndedDelegate, class UGameplayAbility*, AbilityThatEnded, FGameplayAbilitySpecHandle, AbilitySpecHandle, bool, bReplicateEndAbility, bool, bWasCancelled);

UCLASS(config=Game)
class ARPGCharacterBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	/** Our ability system */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities", meta = (AllowPrivateAccess = "true"))
	UAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY()
	class URPGAttributeSetBase* CharacterAttributeSet;

	/** Ref to our player inventory system, since you need to be able to do OwnerPawn->GetInventoryComponent from the ability since the Owner and OwnerPawn are both set to the pawn
	 *this is set in OnRep_PlayerState for the client and Un/OnPossess on server
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	class URPGInventoryComponent* InventoryComponent;

public:
	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseLookUpRate;

	static FName AbilitySystemComponentName;

	static FName CharacterAttributeSetName;

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	ARPGCharacterBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void PostInitializeComponents() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/*InitAbilityActorInfo for the server host (listen server) since this is only called on the server*/
	virtual void PossessedBy(AController* NewController) override;

	/*InitAbilityActorInfo on everything else, see OnRep_PlayerState*/
	virtual void OnRep_Controller() override;

	/*#TODO create an instance of the inventory UI for swapping weapons, so opening and closing does not create and destroy the widget*/
	/*InitAbilityActorInfo for the ability system component on the client, do it here instead of OnRep_Controller since the ability may need the inventory system to be valid*/
	virtual void OnRep_PlayerState() override;

	//IAbilitySystemInterface function
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UFUNCTION(BlueprintCallable)
	virtual URPGAttributeSetBase* GetCharacterAttributeSet();

	UFUNCTION(BlueprintCallable)
	virtual class URPGInventoryComponent* GetInventoryComponent() const { return InventoryComponent; };

protected:
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

	/*input for normal attack*/
	void NormalAttack();


	//////////////////////////////////////////////////////////////////////////
	//INVENTORY
	//////////////////////////////////////////////////////////////////////////

	//how far to trace from the 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	float InteractTraceDistance;

	//how close the item has to be close to the avatar actor to be able to interact with it
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	float InteractDistance;

	//how close the item has to be close to the avatar actor to be able to interact with it
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory", DisplayName = "Inventory UI Class")
	TSubclassOf<class URPGInventoryUI> InventoryUIClass;

	UPROPERTY()
	class URPGInventoryUI* InventoryUI;

	//this is used to check if we have the inventory open each tick, and if so we will check if ItemToEquip is valid and that the distance between the pawn the item to pick up is less
	//than InteractDistance, if we are sliding and we try to pick up an item then we don't want to pick it up when we are too far away
	bool bInventoryUIOpen; 

	//temp var to hold what item we are trying to equip, when the OnSelectSlotInventoryUI is called then we will be equipping this item
	class ARPGInventoryItemBase* ItemToPickup;

	class URPGInventoryUI* CreateInventoryUIWidget(APlayerController* LocalController, class URPGInventoryComponent* InInventoryComponent);

	//do a new line trace and interact with the object that the player is looking at.
	//there should be another trace that runs every tick that checks if the player is looking at any interactive object and display the info
	void Interact();

	/*called when the user select which item to pick up and if they selected a slot, passive items don't need a slot
	 called here since the inventory item need will return an item if swapping and need to be able to place it on the floor next the pawn*/
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_PickupItem(class ARPGInventoryItemBase* Item, ERPGInventorySlot Slot = ERPGInventorySlot::None);

public:
	//callback when the user click on one of the inventory slots
	void OnSelectSlotInventoryUI(ERPGInventorySlot SelectedSlot);

	//callback when the user closes inventory (by clicking close without selecting a slot)
	void OnCloseInventoryUI();

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

public:
	UPROPERTY(BlueprintAssignable)
	FRPGGameplayAbilityEndedDelegate OnAbilityEnded;

	void OnAbilityEnd(const FAbilityEndedData& AbilityEndData);

	/**
	 * Activate the ability using data from inventory component, use the component on character with AI and PlayerState with Players.
	 */
	UFUNCTION(BlueprintCallable, Category = "Abilities")
	bool ActivateAbilitiesWithInputID(ERPGAbilityInputID InputID, bool bAllowRemoteActivation = true);

	//ARPGInventoryItemBase maybe this should be of type RPGACtiveAbilityActor, since all weapons should be of this type
	class ARPGInventoryItemBase* GetCurrentWeapon() const;

	//function called by the MeleeAttackNotifyState, in the character base so that both the player and ai can use the same AnimNotify
	//the ai characters will need to override this in the blueprints
	//this is used for enabling the collision on the current weapon, can be overridden so that the characters arms will have the capsules
	UFUNCTION(BlueprintNativeEvent, Category = "Abilities")
	void OnMeleeAttackStarted();

	//function called by the MeleeAttackNotifyState, in the character base so that both the player and ai can use the same AnimNotify
	//the ai characters will need to override this in the blueprints
	//this is used for disabling the collision on the current weapon, can be overridden so that the characters arms will have the capsules
	UFUNCTION(BlueprintNativeEvent, Category = "Abilities")
	void OnMeleeAttackEnded();
};


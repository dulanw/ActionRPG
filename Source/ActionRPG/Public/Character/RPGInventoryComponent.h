// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "RPGInventoryComponent.generated.h"

//enum used to bind the input to ability
UENUM(BlueprintType)
enum class ERPGInventorySlot : uint8
{
	//Slot none, used for passive items which has no slot
	None				UMETA(DisplayName = "None"),
	//weapon slot, the mesh will be equipped, only 1 weapon slot can be equipped at 1 time which is used for base attacks
	WeaponSlot1			UMETA(DisplayName = "Weapon Slot 1"),
	//weapon slot, the mesh will be equipped, only 1 weapon slot can be equipped at 1 time which is used for base attacks
	WeaponSlot2			UMETA(DisplayName = "Weapon Slot 2"),
	//ability slot, these are ability that are invoked but the model cannot be equipped
	ActiveItemSlot1		UMETA(DisplayName = "Active Item Slot 1"),
	//ability slot, these are ability that are invoked but the model cannot be equipped
	ActiveItemSlot2		UMETA(DisplayName = "Active Item Slot 2")
};

UENUM(BlueprintType)
enum class ERPGAbilityInputID : uint8
{	
	None				UMETA(DisplayName = "None"),
	// 1 LMB
	PrimaryFire			UMETA(DisplayName = "PrimaryFire"),
	// 2 RMB
	SecondaryFire		UMETA(DisplayName = "SecondaryFire"),
	// 3 Q
	Ability1			UMETA(DisplayName = "Ability1"),
	// 4 E
	Ability2			UMETA(DisplayName = "Ability2")
};

USTRUCT(BlueprintType)
struct FRPGInventorySlotData
{
	GENERATED_BODY()

	//the current slot
	UPROPERTY()
	ERPGInventorySlot Slot;

	//Current actor, when picked up need to be assigned alongside the ability spec handle and removed when dropped
	//some might not have an actor but most should, since you are able to pick up and drop items (don't delete and respawn, just hide it)
	UPROPERTY()
	class ARPGInventoryItemBase* ItemActor;

	FRPGInventorySlotData()
		: Slot(ERPGInventorySlot::None), ItemActor(nullptr)
	{

	}

	FRPGInventorySlotData(ERPGInventorySlot InSlot, class ARPGInventoryItemBase* InItemActor)
		: Slot(InSlot), ItemActor(InItemActor)
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

};

/*Ability slot handle, these are separate from the actual inventory item, since they will need to be recreated from the actual inventory slot actors when the owning actor dies
 this is mainly used for getting which slot the ability is in, then for getting cool down tags
*/
USTRUCT(BlueprintType)
struct FRPGAbilityInputHandleData
{
	GENERATED_BODY()

	//the current slot
	UPROPERTY()
	ERPGAbilityInputID InputID;

	//the current ability handle
	UPROPERTY()
	FGameplayAbilitySpecHandle AbilitySpecHandle;

	FRPGAbilityInputHandleData()
		: InputID(ERPGAbilityInputID::None), AbilitySpecHandle(FGameplayAbilitySpecHandle())
	{

	}

	FRPGAbilityInputHandleData(ERPGAbilityInputID InInputID, FGameplayAbilitySpecHandle InAbilitySpecHandle)
		: InputID(InInputID), AbilitySpecHandle(InAbilitySpecHandle)
	{

	}

	bool operator==(const FRPGAbilityInputHandleData& Other) const
	{
		return (InputID == Other.InputID);
	}

	bool operator==(const ERPGAbilityInputID& InInputID) const
	{
		return (InputID == InInputID);
	}

	bool operator==(const FGameplayAbilitySpecHandle& InAbilitySpecHandle) const
	{
		return (InAbilitySpecHandle == AbilitySpecHandle);
	}
};

/**
 * Inventory component, will be held in the player state so that the player item's can be visible to other players
 * Need to set the ability system component owner since that is the one used to grant the abilities etc..
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ACTIONRPG_API URPGInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

	/*contains the abilities for the inventory slots, i.e only usable abilities and weapons, primary and secondary weapon included at the same time
	 *use a variable i.e. CurrentWeaponSlot = weaponslot1 etc..
	 *using a TArray instead of TMap since this needs to be replicated, use .Find with FRPGInventorySlot to find the corresponding ability and actor or
	 *use the ability handle to find which slot it's occupying, which then you can use to find the cool down tag on SlotCooldownTags etc
	 */
	UPROPERTY(ReplicatedUsing = OnRep_Inventory)
	TArray<FRPGInventorySlotData> SlottedInventory;

	/*Inventory items that don't have a specific slot, and free, just includes a list of Inventory Items.
	 *if they grant a ability you are not able to locate which item gave it except for the source object
	 */
	UPROPERTY(ReplicatedUsing = OnRep_Inventory)
	TArray<ARPGInventoryItemBase*> LooseInventory;

	//the currently selected weapon slot, should only be either weapon slot 1 or weapon slot 2, cannot select any ability slots
	//use switch weapons to change this
	//UPROPERTY(ReplicatedUsing = OnRep_ActiveWeaponSlot)
	//ERPGInventorySlot ActiveWeaponSlot;

	//use a weapon pointer, because when the active weapon slot is replicated, the inventory system may not have been fully replicated
	UPROPERTY(ReplicatedUsing = OnRep_CurrentWeapon)
	class ARPGInventoryItemBase* CurrentWeapon;

	//#TODO bIsEquipping, no need to have a variable for each weapon


public:	
	// Sets default values for this component's properties
	URPGInventoryComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	static TSet<ERPGInventorySlot> GetCompatibleSlotsByItem(class ARPGInventoryItemBase* Item);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	class ARPGInventoryItemBase* GetCurrentWeapon() { return CurrentWeapon; };

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	/*The ability handles that are created when item's are added to the slotted inventory*/
	UPROPERTY(Replicated)
	TArray<FRPGAbilityInputHandleData> AbilityInputHandles;

	//the cool down tag corresponding to each ability slot, if the tag or slot is empty from this list then there will be no cool down for that slot
	//primary and secondary fire will not have cool downs since you should be able to swap weapons and fire instantly
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	TMap<ERPGAbilityInputID, FGameplayTagContainer> AbilityInputCooldownTags;

	/** Called on owning client when SlottedInventory or LooseInventory is replicated.*/
	UFUNCTION()
	virtual void OnRep_Inventory();

	/** Called on owning client when ActiveWeaponSlot is replicated.*/
	UFUNCTION()
	virtual void OnRep_CurrentWeapon(class ARPGInventoryItemBase* LastWeapon);

	//check if player or ai, if ai then get the owner straight away since it would be the pawn, if not get the pawn from the player state
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const;

	/*add an item to a slot, you should remove an item before adding, otherwise the actor might be lost if not dropped before
	 *the actor might be null since you might be given an ability without a source object but should be very rare
	 *this will remove any abilities in the slot, although you should call RemoveItemFromSlot which will remove the ability anyway
	 *#TODO return true or false depending on whether the ability was added or not, 
	 *it might not have been added if the player is dead if the inventory is on the player state and the ability system is on the pawn, so will need to refresh the inventory
	 */
	void BindAbilityToInput(ERPGAbilityInputID InputID, TSubclassOf<UGameplayAbility> AbilityToAcquire, AActor* SourceObject = nullptr);

	/*Add an ability to the user with sourceObject
	 *@return the handle, or handle with INDEX_NONE if invalid GameplayAbility
	 *#TODO pass in level, and other values etc...
	 */
	FGameplayAbilitySpecHandle AcquireAbility(TSubclassOf<UGameplayAbility> AbilityToAcquire, AActor* SourceObject = nullptr);

	//remove ability bound to a specific input
	void UnbindAbilityFromInput(ERPGAbilityInputID InputID);

	/*Get the Input matched to the inventory slot
	 *@param bSecondaryAbility if this is the secondary ability, since weapons will have 2 slots. 
	 *@return the input id for the slot, this is mainly used for BindAbilityToInput
	 */
	ERPGAbilityInputID GetInputIDFromSlot(ERPGInventorySlot Slot, bool bSecondaryAbility = false) const;

	//ItemType must be weapon, should call SwitchWeapons with the inventory slot which will call EquipWeapon
	virtual void EquipWeapon(class ARPGInventoryItemBase* Weapon);

	/** equip weapon */
	UFUNCTION(Reliable, Server, WithValidation)
	void Server_EquipWeapon(class ARPGInventoryItemBase* Weapon);

	/** updates current weapon, will unequip the last weapon if given a valid ptr, if NewWeapon does not equal the current weapon then the current weapon will be unequipped
	 * the LastWeapon should only be used with OnRep_Current weapon, since when we equip a new weapon, the client should unequip the previous weapon
	 */
	void SetCurrentWeapon(class ARPGInventoryItemBase* NewWeapon, class ARPGInventoryItemBase* LastWeapon = nullptr);

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/**
	 * Add an item to the inventory
	 * @param Item item to add to inventory
	 * @param Slot the slot to which the item can be activated from, will fail the item is an active ability which requires a slot and no slot is provided or if there is already an existing item
	 * @return return true if the item is added, if the slot is invalid or occupied then it will return false, should call RemoveItemFromSlot before calling this
	 */
	bool AddItem(class ARPGInventoryItemBase* Item, ERPGInventorySlot Slot = ERPGInventorySlot::None);

	/**
	 * remove an item from a given slot
	 * @param Slot the slot to remove the item from, should be anything other than SLOT_NONE
	 * @return the item returned, nothing is destroyed, only removed from inventory
	 */
	class ARPGInventoryItemBase* RemoveItemFromSlot(ERPGInventorySlot Slot);
	
	//#TODO remove item from passive abilities?? not needed right now
	//bool RemoveItem();

	//get the slotted inventory list
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	TArray<FRPGInventorySlotData> GetSlottedInventory() { return SlottedInventory; }

	//Get the item in the inventory slot
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	class ARPGInventoryItemBase* GetSlotInventoryItem(ERPGInventorySlot Slot) const;

	//get the loose inventory list
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	TArray<ARPGInventoryItemBase*> GetLooseInventory() { return LooseInventory; }

	//Get the slot which the gameplay ability is slotted to. used to find the cool down tag for the slot
	//#TODO  maybe use the InputID already in the ability spec? unless setting that will cause issues
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	ERPGAbilityInputID GetAbilityHandleInputID(const FGameplayAbilitySpecHandle& AbilitySpecHandle) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FGameplayTagContainer GetAbilityInputCooldownTag(const ERPGAbilityInputID& InputID) const;

	/**
	 * Attempts to activate any ability in the specified item slot. Will return false if no activatable ability found or activation fails
	 * Returns true if it thinks it activated, but it may return false positives due to failure later in activation.
	 * If bAllowRemoteActivation is true, it will remotely activate local/server abilities, if false it will only try to locally activate the ability
	 */
	bool ActivateAbilitiesWithInputID(ERPGAbilityInputID InputID, bool bAllowRemoteActivation = true);
};

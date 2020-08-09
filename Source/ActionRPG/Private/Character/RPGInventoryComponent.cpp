// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/RPGInventoryComponent.h"
#include "Character/RPGCharacterBase.h"
#include "Items/RPGInventoryItemBase.h"
#include "AbilitySystemInterface.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/PlayerState.h"

// Sets default values for this component's properties
URPGInventoryComponent::URPGInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);

	//default slot cool downs, weapons will not share slot cool down
	//SlotCooldownTags.Add(ERPGInventorySlot::WeaponSlot1, FGameplayTagContainer(FGameplayTag::RequestGameplayTag(FName("Cooldown.Slot.WeaponSlot1"))));
	//SlotCooldownTags.Add(ERPGInventorySlot::WeaponSlot2, FGameplayTagContainer(FGameplayTag::RequestGameplayTag(FName("Cooldown.Slot.WeaponSlot2"))));
	AbilityInputCooldownTags.Add(ERPGAbilityInputID::Ability1, FGameplayTagContainer(FGameplayTag::RequestGameplayTag(FName("Cooldown.Input.Ability1"))));
	AbilityInputCooldownTags.Add(ERPGAbilityInputID::Ability2, FGameplayTagContainer(FGameplayTag::RequestGameplayTag(FName("Cooldown.Input.Ability1"))));
}


void URPGInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(URPGInventoryComponent, SlottedInventory);
	DOREPLIFETIME(URPGInventoryComponent, LooseInventory);
	DOREPLIFETIME(URPGInventoryComponent, CurrentWeapon);
	DOREPLIFETIME_CONDITION(URPGInventoryComponent, AbilityInputHandles, COND_OwnerOnly);
}

TSet<ERPGInventorySlot> URPGInventoryComponent::GetCompatibleSlotsByItem(ARPGInventoryItemBase* Item)
{
	TSet<ERPGInventorySlot> CompatibleSlots;

	if (Item)
	{
		const ERPGItemType ItemType = Item->ItemType;

		switch (ItemType)
		{
		case ERPGItemType::ActiveItem:
			CompatibleSlots.Add(ERPGInventorySlot::ActiveItemSlot1);
			CompatibleSlots.Add(ERPGInventorySlot::ActiveItemSlot2);
			break;
		case ERPGItemType::Weapon:
			CompatibleSlots.Add(ERPGInventorySlot::WeaponSlot1);
			CompatibleSlots.Add(ERPGInventorySlot::WeaponSlot2);
			break;
		case ERPGItemType::PassiveItem:
		case ERPGItemType::None:
		default:
			break;
		}
	}

	return CompatibleSlots;
	
}

void URPGInventoryComponent::OnRep_Inventory()
{
	//#TODO call swap weapons or get the best next weapon, since the inventory might have dropped or swapped our current weapon
}

void URPGInventoryComponent::OnRep_CurrentWeapon(ARPGInventoryItemBase* LastWeapon)
{
	SetCurrentWeapon(CurrentWeapon, LastWeapon);
}

UAbilitySystemComponent* URPGInventoryComponent::GetAbilitySystemComponent() const
{
	//#TODO check if the inventory is in the player state, if so then get the pawn from player state
	APlayerState* PlayerState = Cast<APlayerState>(GetOwner());

	AActor* AbilitySystemOwner = (PlayerState) ? PlayerState->GetPawn() : GetOwner(); //if the inventory isn't on the player state then we assume both components are on the same actor
	IAbilitySystemInterface* AbilitySystemIntf = Cast<IAbilitySystemInterface>(AbilitySystemOwner);
	
	return (AbilitySystemIntf) ? AbilitySystemIntf->GetAbilitySystemComponent() : nullptr;
}

void URPGInventoryComponent::BindAbilityToInput(ERPGAbilityInputID InputID, TSubclassOf<UGameplayAbility> AbilityToAcquire, AActor* SourceObject /*= nullptr*/)
{
	//can't do anything without the ability system or it's not authority or we don't have an ability
	//even if it's empty we should unbind the ability anyway, just to clean it up
	if (!GetAbilitySystemComponent() || !GetOwner()->HasAuthority() /*|| !AbilityToAcquire*/)
	{
		return;
	}

	//unbind any bound abilities
	UnbindAbilityFromInput(InputID);

	if (AbilityToAcquire)
	{
		FGameplayAbilitySpecHandle NewAbilitySpecHandle = AcquireAbility(AbilityToAcquire, SourceObject);
		if (NewAbilitySpecHandle.IsValid())
		{
			AbilityInputHandles.Add(FRPGAbilityInputHandleData(InputID, NewAbilitySpecHandle));
		}
	}
}

FGameplayAbilitySpecHandle URPGInventoryComponent::AcquireAbility(TSubclassOf<UGameplayAbility> AbilityToAcquire, AActor* SourceObject /*= nullptr*/)
{
	//need to do a check again here since loosely added inventory items will call AcquireAbility without going through AddAbilityToSlot
	if (!GetAbilitySystemComponent() || !GetOwner()->HasAuthority() || !AbilityToAcquire)
	{
		return FGameplayAbilitySpecHandle();
	}

	//pass INDEX_NONE for InputID since we aren't using the input mapping
	return GetAbilitySystemComponent()->GiveAbility(FGameplayAbilitySpec(AbilityToAcquire, 1, INDEX_NONE, SourceObject));
}

void URPGInventoryComponent::UnbindAbilityFromInput(ERPGAbilityInputID InputID)
{
	//need to do a check again here since both RemoveItemFromSlot and AddAbilityToSlot calls this to clear any existing abilities in the slot
	FRPGAbilityInputHandleData* FoundInputData = AbilityInputHandles.FindByKey(InputID);

	if (FoundInputData)
	{
		//remove the ability previous ability, it maybe that the player pawn is dead and if we have the inventory component on the player state, then the component will be destroyed when the player dies
		//so it might not exist at this point so we don't even have to remove the component
		if (GetAbilitySystemComponent())
		{
			GetAbilitySystemComponent()->ClearAbility(FoundInputData->AbilitySpecHandle);
		}

		//remove the index, just much easier than modifying the data depending on if we found it or not
		AbilityInputHandles.Remove(*FoundInputData); //#TODO use RemoveAll, currently using a pointer to the data to we are about to remove, might cause issues if we have to remove multiple
	}
}

ERPGAbilityInputID URPGInventoryComponent::GetInputIDFromSlot(ERPGInventorySlot Slot, bool bSecondaryAbility /*= false*/) const
{
	switch (Slot)
	{
		case ERPGInventorySlot::WeaponSlot1:
		case ERPGInventorySlot::WeaponSlot2:
			return (bSecondaryAbility) ? ERPGAbilityInputID::SecondaryFire : ERPGAbilityInputID::PrimaryFire;

		case ERPGInventorySlot::ActiveItemSlot1:
			return ERPGAbilityInputID::Ability1;

		case ERPGInventorySlot::ActiveItemSlot2:
			return ERPGAbilityInputID::Ability2;

		default:
			return ERPGAbilityInputID::None;
	}
}

// Called when the game starts
void URPGInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...

}

// Called every frame
void URPGInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

bool URPGInventoryComponent::AddItem(ARPGInventoryItemBase* Item, ERPGInventorySlot Slot /*= ERPGInventorySlot::None*/)
{
	if (!GetOwner()->HasAuthority() || !Item || Item->ItemType > ERPGItemType::PassiveItem && Slot == ERPGInventorySlot::None)
	{
		UE_LOG(LogTemp, Error, TEXT("URPGInventoryComponent::AddItem: No Authority or Invalid Item/Slot"));
		return false;
	}

	//if we require slot then we need to make sure it's not already occupied
	if (Item->ItemType > ERPGItemType::PassiveItem)
	{
		const ARPGInventoryItemBase* FoundItem = GetSlotInventoryItem(Slot);

		if (FoundItem)
		{
			//Don't call swap item from here since we need to get a pointer to the swapped weapon which we would place on the ground
			UE_LOG(LogTemp, Error, TEXT("URPGInventoryComponent::AddItem: Slot contains item already, remove item from slot before adding or call SwapItem"));
			return false;
		}

		SlottedInventory.Add(FRPGInventorySlotData(Slot, Item)); //add the item to the inventory
		Item->OnEnterInventory(GetOwner());

		//ideally if we have 2 weapons and we are picking up one then we should have called remove item from slot at which point the ActiveWeapon slot would be set to the other weapon slot
		//so they should not be equal #TODO maybe add SwapItem slot instead???
		if (Item->ItemType != ERPGItemType::Weapon)
		{
			BindAbilityToInput(GetInputIDFromSlot(Slot), Item->PrimaryGameplayAbilityGranted, Item);
		}
		else if(Item->ItemType == ERPGItemType::Weapon && !CurrentWeapon) //if we are not holding a weapon already 
		{
			//#TODO find a better way instead of having primary and secondary abilities (maybe an array)
			BindAbilityToInput(GetInputIDFromSlot(Slot), Item->PrimaryGameplayAbilityGranted, Item);
			BindAbilityToInput(GetInputIDFromSlot(Slot, true), Item->SecondaryGameplayAbilityGranted, Item);
			EquipWeapon(Item);
		}

		return true;
	}
/*
	else if (Item->ItemType != ERPGItemType::None)
	{
		//#TODO check if we have the item already in our inventory, if so then we should just increment it
	}*/

	return false;
}

ARPGInventoryItemBase* URPGInventoryComponent::RemoveItemFromSlot(ERPGInventorySlot Slot)
{
	//need to be authority and have a valid slot, currently not able to remove loosely added inventory items
	if (!GetOwner()->HasAuthority() || Slot == ERPGInventorySlot::None)
	{
		return nullptr;
	}

	ARPGInventoryItemBase* FoundItem = nullptr;
	const FRPGInventorySlotData* FoundSlotData = SlottedInventory.FindByKey(Slot);
	if (FoundSlotData)
	{
		FoundItem = FoundSlotData->ItemActor;
		SlottedInventory.Remove(*FoundSlotData); //#TODO use RemoveAll, currently using a pointer to the data to we are about to remove, might cause issues if we have to remove multiple
	}



	//remove ability from slot, if there is no valid item then there shouldn't be an ability either
	//UnbindAbilityFromInput(Slot); #TODO remove the weapon ability if it's the current one equipped, if not then keep it

	return FoundItem;
}

ARPGInventoryItemBase* URPGInventoryComponent::GetSlotInventoryItem(ERPGInventorySlot Slot) const
{
	const FRPGInventorySlotData* FoundSlotData = SlottedInventory.FindByKey(Slot);

	if (FoundSlotData)
	{
		return FoundSlotData->ItemActor;
	}

	return nullptr;
}

ERPGAbilityInputID URPGInventoryComponent::GetAbilityHandleInputID(const FGameplayAbilitySpecHandle& AbilitySpecHandle) const
{
	const FRPGAbilityInputHandleData* const FoundInputData = AbilityInputHandles.FindByKey(AbilitySpecHandle);

	if (FoundInputData)
	{
		return FoundInputData->InputID;
	}

	return ERPGAbilityInputID::None;
}

FGameplayTagContainer URPGInventoryComponent::GetAbilityInputCooldownTag(const ERPGAbilityInputID& InputID) const
{
	const FGameplayTagContainer* CooldownTag = AbilityInputCooldownTags.Find(InputID);

	if (CooldownTag)
	{
		return *CooldownTag;
	}

	return FGameplayTagContainer();
}

bool URPGInventoryComponent::ActivateAbilitiesWithInputID(ERPGAbilityInputID InputID, bool bAllowRemoteActivation /*= true*/)
{
	//Find returns a pointer, so make sure to dereference it
	FRPGAbilityInputHandleData* FoundInputData = AbilityInputHandles.FindByKey(InputID);

	//make sure the slot data is found and the ability spec handle is valid
	if (GetAbilitySystemComponent() && FoundInputData && FoundInputData->AbilitySpecHandle.IsValid())
	{
		return GetAbilitySystemComponent()->TryActivateAbility(FoundInputData->AbilitySpecHandle, bAllowRemoteActivation);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("URPGInventoryComponent::ActivateAbilitiesWithInputID: ability component/handle NOT found, Make sure item/ability was binded to input ID"));
	}

	return false;
}

void URPGInventoryComponent::EquipWeapon(ARPGInventoryItemBase* Weapon)
{
	if (Weapon && Weapon->ItemType == ERPGItemType::Weapon)
	{
		if (GetOwner()->GetLocalRole() == ROLE_Authority)
		{
			SetCurrentWeapon(Weapon, CurrentWeapon);
		}
		else
		{
			Server_EquipWeapon(Weapon);
		}
	}
}

void URPGInventoryComponent::Server_EquipWeapon_Implementation(ARPGInventoryItemBase* Weapon)
{
	EquipWeapon(Weapon);
}

bool URPGInventoryComponent::Server_EquipWeapon_Validate(ARPGInventoryItemBase* Weapon)
{
	return true;
}

void URPGInventoryComponent::SetCurrentWeapon(ARPGInventoryItemBase* NewWeapon, ARPGInventoryItemBase* LastWeapon /*= nullptr*/)
{
	ARPGInventoryItemBase* LocalLastWeapon = LastWeapon;

	if (NewWeapon != CurrentWeapon)
	{
		LocalLastWeapon = CurrentWeapon;
	}

	// unequip previous
	if (LocalLastWeapon)
	{
		LocalLastWeapon->OnUnEquip();
	}

	CurrentWeapon = NewWeapon;

	// equip new one
	if (NewWeapon)
	{
		//NewWeapon->GiveTo(GetOwner());	// Make sure weapon's MyPawn is pointing back to us. During replication, we can't guarantee APawn::CurrentWeapon will rep after AWeapon::MyPawn!
	
		AActor* AvatarActor = GetAbilitySystemComponent()->GetAvatarActor();
		ARPGCharacterBase* AvatarCharacter = Cast<ARPGCharacterBase>(AvatarActor);
		if (AvatarCharacter)
		{
			NewWeapon->OnEquip(AvatarCharacter, GetOwner());
		}
	}
}

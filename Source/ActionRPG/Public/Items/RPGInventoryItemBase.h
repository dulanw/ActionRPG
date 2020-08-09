// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RPGInventoryItemBase.generated.h"

UENUM()
enum class ERPGItemType : uint8
{
	//None is same as passive item as it will be added to the passive item list but will not grant any abilities
	None				UMETA(DisplayName = "None"),
	//will be added to the passive item list and the primary ability will be granted, not bound to any player input so the ability must take care of activation
	PassiveItem			UMETA(DisplayName = "Passive Item"),
	//added to a slot, into one of the specific slots
	ActiveItem			UMETA(DisplayName = "Active Item"),
	//weapon, must be equipped at which point the primary and secondary abilities will be bound to the primary and secondary fire user inputs
	Weapon				UMETA(DisplayName = "Weapon")
};

UCLASS(abstract)
class ACTIONRPG_API ARPGInventoryItemBase : public AActor
{
	GENERATED_BODY()

	/** the mesh shown when the item is dropped on the ground */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item", meta = (AllowPrivateAccess = "true"))
	class USkeletalMeshComponent* Mesh;

	/** turn table effect for dropped items. when items are dropped, the mesh will be attached to this position and keep the relative rotation and location to the root
	 * the actor will be rotating while dropped
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (AllowPrivateAccess = "true"))
	class USceneComponent* TurntableAttachOffset;

	//when this actor is attached to the hands, the mesh will be attached to this offset point,
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (AllowPrivateAccess = "true"))
	class USceneComponent* EquippedAttachOffset;

	/*Collision for the interactive sphere */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item", meta = (AllowPrivateAccess = "true"))
	class USphereComponent* InteractiveCollision;
	
public:	
	// Sets default values for this actor's properties
	ARPGInventoryItemBase(const FObjectInitializer& ObjectInitializer);

	/** Returns the properties used for network replication, this needs to be overridden by all actor classes with native replicated properties */
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	USkeletalMeshComponent* GetMesh() { return Mesh; }

protected:

	//is the item equipped on the owner pawn, this is set instead of getting if CurrentWeapon == this when we replicate the owner to the client
	//since the owner might be getting replicated after the client gets the CurrentWeapon replicated
	bool bIsEquipped;

	//the avatar that the weapon is attached to, set in OnEquip
	//used in AttachmeshToPawn, this is also used for the collision etc, since the owner can be a player state
	//UPROPERTY(ReplicatedUsing = OnRep_AvatarCharacter)
	class ARPGCharacterBase* AvatarCharacter;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//use OnRep_Owner to know if the item was added the inventory or not
	virtual void OnRep_Owner() override;

	//UFUNCTION()
	//virtual void OnRep_AvatarCharacter();

	//Give the item to a new owner, set the owner and instigator, should only be called by the server but will be called on the client when the current weapon is replicated
	//to make sure that we always have the latest owner on the client
	//made AActor from APawn, since the owner of the inventory component will be the owner which could be the PlayerState or PlayerController in the future
	//we can give the item with a new avatar actor or set it later, when are creating default items, the players pawn might not exist so we need to call UpdateInventoryAvatar or something
	//void GiveTo(class AActor* NewOwner, class ACharacter* NewAvatarCharacter = nullptr);

	void AttachMeshToPawn();

	void DetachMeshFromPawn();

public:	

	//the item types, depending on what the item type is, it will be added to a different slot or the passive item list.
	//if none then it will be added to the passive list as well but will not grant an ability
	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	ERPGItemType ItemType;

	//the ability granted when this item is equipped or added to the passive item list
	UPROPERTY(EditDefaultsOnly, Category = "Ability", meta = (EditCondition = "ItemType != ERPGItemType::None"))
	TSubclassOf<class UGameplayAbility> PrimaryGameplayAbilityGranted;

	//Secondary fire ability when this weapon is equipped, must be a weapon since only weapons grant secondary abilities
	UPROPERTY(EditDefaultsOnly, Category = "Ability", meta = (EditCondition = "ItemType == ERPGItemType::Weapon"))
	TSubclassOf<class UGameplayAbility> SecondaryGameplayAbilityGranted;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//function called when item is picked up and put into inventory, this is also called when the owner is replicated depending on the conditions
	//#TODO might not have to call this again from the inventory component since we are using the owner to check if we have picked it up or not
	//so calling OnPickup from GiveTo might work as well
	//we pass in the instigator as well, since the item should have a ref to the pawn since the owner could be the player state/controller and not a pawn
	virtual void OnEnterInventory(class AActor* NewOwner, class ARPGCharacterBase* NewAvatarCharacter = nullptr);

	//function called when item is dropped from inventory
	//#TODO might not have to call this again from the inventory component since we are using the owner to check if we have picked it up or not
	//so calling OnDrop from GiveTo might work as well
	virtual void OnLeaveInventory();

	/** weapon is being equipped by owner pawn 
	 * when the item is picked up, the owner of the item doesn't have to be a pawn, it could be the player state
	 * and the item can only be equipped if there is a character to equip it to
	 * since the avatar actor and the owner can be different, pass in the new owner and the avatar actor 
	 * might be better to call GiveTo separately instead of passing in both.
	 */
	virtual void OnEquip(class ARPGCharacterBase* NewAvatarCharacter, class AActor* NewOwner = nullptr);

	/** weapon is now equipped by owner pawn */
	virtual void OnEquipFinished(class ARPGCharacterBase* NewAvatarCharacter);

	/** weapon is in inventory*/
	virtual void OnUnEquip();
};

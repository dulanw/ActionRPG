// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Character/RPGInventoryComponent.h"
#include "RPGInventoryUI.generated.h"

//this is for the inventory when swapping or picking up items
DECLARE_MULTICAST_DELEGATE_OneParam(FRPGInventorySlotSelectSignature, ERPGInventorySlot); 

//this is for closing the inventory tab without selecting an item slot when swapping or picking up items, can also call FRPGInventorySlotSelectSignature but with ERPGInventorySlot::None
DECLARE_MULTICAST_DELEGATE(FRPGCloseInventorySignature); 

/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class ACTIONRPG_API URPGInventoryUI : public UUserWidget
{
	GENERATED_BODY()
	
public:

	FRPGInventorySlotSelectSignature OnInventorySelectSlot;

	FRPGCloseInventorySignature OnInventoryClose;

	URPGInventoryUI(const FObjectInitializer& ObjectInitializer);

	void InitializeWidget(URPGInventoryComponent* OwnerInventory);

	/**
	 * show the widget, adds to the OwningPlayer, should be set in CreateWidget(PC,...) 
	 *@param ItemToEquip if we are trying to equipping an item then it must be passed into here, then in the widget blueprint you need to check which slots that it can be equipped onto
	 *if there is none then that means it's VisibleOnly and the user won't be able change anything
	 */
	void Show(class ARPGInventoryItemBase* InItemToEquip = nullptr, int32 ZOrder = 0);

	void Hide();

	/**
	 * refresh the ui, if equipping item then the item need to be passed into ItemToEquip, must InitializeInventory
	 */
	void Refresh();

protected:
	UPROPERTY(BlueprintReadOnly)
	class URPGInventoryComponent* OwnerInventoryComponent;

	//temp variable to hold when equipping an item, should be set in Show() and cleared in Hide(), #TODO pass this value OnInventorySlotClicked ?? instead of storing it again on the pawn??
	UPROPERTY(BlueprintReadOnly)
	class ARPGInventoryItemBase* TempItemToEquip;

	//c++ wrapper for OnInventorySelectSlot.broadcast from blueprint
	UFUNCTION(BlueprintCallable)
	void OnInventorySlotClicked(ERPGInventorySlot SelectedSlot);

	//c++ wrapper for OnInventoryClose.broadcast from blueprint, only called when inventory is closed when picking up items and the X is shown
	UFUNCTION(BlueprintCallable)
	void OnInventoryClosed();

	bool bHasBlueprintRefresh;

	UFUNCTION(BlueprintImplementableEvent, DisplayName = "Refresh", meta = (ScriptName = "Refresh"))
	void K2_Refresh();

};

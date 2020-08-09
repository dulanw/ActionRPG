// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/RPGInventoryUI.h"
#include "Character/RPGInventoryComponent.h"
#include "Items/RPGInventoryItemBase.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

URPGInventoryUI::URPGInventoryUI(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	auto ImplementedInBlueprint = [](const UFunction* Func) -> bool
	{
		return Func && ensure(Func->GetOuter())
			&& (Func->GetOuter()->IsA(UBlueprintGeneratedClass::StaticClass()) || Func->GetOuter()->IsA(UDynamicClass::StaticClass()));
	};

	{
		static FName FuncName = FName(TEXT("K2_Refresh"));
		UFunction* ShouldRespondFunction = GetClass()->FindFunctionByName(FuncName);
		bHasBlueprintRefresh = ImplementedInBlueprint(ShouldRespondFunction);
	}

	TempItemToEquip = nullptr;
	OwnerInventoryComponent = nullptr;
}

void URPGInventoryUI::InitializeWidget(URPGInventoryComponent* OwnerInventory)
{
	OwnerInventoryComponent = OwnerInventory;
}

void URPGInventoryUI::Show(ARPGInventoryItemBase* InItemToEquip /*= nullptr*/, int32 ZOrder /*= 0*/)
{
	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		TempItemToEquip = InItemToEquip;
		Refresh();

		SetVisibility(ESlateVisibility::Visible);
		AddToViewport(ZOrder);
		UWidgetBlueprintLibrary::SetInputMode_UIOnlyEx(PC, this, EMouseLockMode::LockAlways); //PC->SetInputMode(FInputModeUIOnly)
		PC->bShowMouseCursor = true;
	}
}

void URPGInventoryUI::Hide()
{
	SetVisibility(ESlateVisibility::Collapsed);

	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		UWidgetBlueprintLibrary::SetInputMode_GameOnly(PC); //PC->SetInputMode(FInputModeUIOnly) 
		RemoveFromViewport();	
		PC->bShowMouseCursor = false;
	}
}

void URPGInventoryUI::Refresh()
{
	if (bHasBlueprintRefresh)
	{
		K2_Refresh();
	}
}

void URPGInventoryUI::OnInventorySlotClicked(ERPGInventorySlot SelectedSlot)
{
	OnInventorySelectSlot.Broadcast(SelectedSlot);
}

void URPGInventoryUI::OnInventoryClosed()
{
	OnInventoryClose.Broadcast();
}

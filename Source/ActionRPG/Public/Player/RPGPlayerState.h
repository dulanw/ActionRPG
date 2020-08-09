// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "RPGPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class ACTIONRPG_API ARPGPlayerState : public APlayerState
{
	GENERATED_BODY()

	UPROPERTY()
	class URPGInventoryComponent* InventoryComponent;

public:
	ARPGPlayerState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	class URPGInventoryComponent* GetPlayerInventoryComponent() { return InventoryComponent; }
};

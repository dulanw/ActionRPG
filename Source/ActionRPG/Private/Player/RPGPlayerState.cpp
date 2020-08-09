// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/RPGPlayerState.h"
#include "Character/RPGInventoryComponent.h"

ARPGPlayerState::ARPGPlayerState(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	: Super(ObjectInitializer)
{
	InventoryComponent = CreateDefaultSubobject<URPGInventoryComponent>(TEXT("InventoryComponent"));
}

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "RPGEngineSubsystem.generated.h"

/**
 * base inventory ui, used for displaying the inventory when pressing TAB and when picking up items. need to pass in a bool when opening
 */
UCLASS()
class ACTIONRPG_API URPGEngineSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
};

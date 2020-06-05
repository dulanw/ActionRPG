// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "RPGPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class ACTIONRPG_API ARPGPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	virtual void AcknowledgePossession(class APawn* P) override;
};

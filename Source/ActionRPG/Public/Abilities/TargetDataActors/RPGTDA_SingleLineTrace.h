// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetActor_Trace.h"
#include "RPGTDA_SingleLineTrace.generated.h"

/**
 * 
 */
UCLASS(notplaceable)
class ACTIONRPG_API ARPGTDA_SingleLineTrace : public AGameplayAbilityTargetActor_Trace
{
	GENERATED_BODY()
public:
	ARPGTDA_SingleLineTrace(const FObjectInitializer& ObjectInitializer);

	//need to override this since AGameplayAbilityTargetActor_Trace set bDebug to false here always
	virtual void ConfirmTargetingAndContinue() override;

protected:
	virtual FHitResult PerformTrace(AActor* InSourceActor) override; //#TODO right now it's doing the trace from the overhead camera, do another trace from the character pos/weapon muzzle to hit
};

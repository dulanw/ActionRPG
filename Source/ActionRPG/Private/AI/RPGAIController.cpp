// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/RPGAIController.h"
#include "AI/RPGPathFollowingComponent.h"

ARPGAIController::ARPGAIController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<URPGPathFollowingComponent>(TEXT("PathFollowingComponent")))
{

}

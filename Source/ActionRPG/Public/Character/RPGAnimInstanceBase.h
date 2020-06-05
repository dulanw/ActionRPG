// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "RPGAnimInstanceBase.generated.h"

/**
 * 
 */
UCLASS()
class ACTIONRPG_API URPGAnimInstanceBase : public UAnimInstance
{
	GENERATED_BODY()

public:
	URPGAnimInstanceBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	// Native initialization override point
	virtual void NativeInitializeAnimation() override;

	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:

	class ACharacter* OwnerCharacter;

	//used only for fixing the issue with switching direction
	//can go from -180 -> 180 (when you are moving forward) or 0 -> 360 (when you are moving backward)
	//This is the BlendSpace Direction we use
	UPROPERTY(BlueprintReadOnly, Category="Locomotion")
	float LocomotionBSDirection;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	bool bLocomotionMovingForward;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	float LocomotionPlayRate;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	float MoveSpeed;

	//UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	//bool bIsMovingForward;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	bool bIsMoving;

	//the move speed at which anim matches the movement, i.e when it's playing at a 1 play rate
	//default is 600 since that's the default max speed, so at 600 it will have a play rate of 600 (basically movespeed * 1/600)
	UPROPERTY(EditDefaultsOnly, Category = "Locomotion")
	float BlendSpaceDefaultMoveSpeed;

	//used for checking for intial jump, use is falling or is in air to check for first jump
	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	bool bIsInAir;

	//used for checking for double jump, use is falling or is in air to check for first jump
	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	bool bDoubleJumped;

	//keep track of the jump count, if the number of jump count is different between frames that means the char jumped (only if it increases)
	int32 JumpCount;

	void UpdateLocomotionVars(float DeltaSeconds);

	//ANIMATION OVERRIDES FOR CHARACTER/WEAPON
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Upper Body")
	class UAnimSequence* UpperBodyIdleAnim;
};

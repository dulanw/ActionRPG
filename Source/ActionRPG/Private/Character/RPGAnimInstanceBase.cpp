// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/RPGAnimInstanceBase.h"
#include "Character/RPGCharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"

URPGAnimInstanceBase::URPGAnimInstanceBase(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	: Super(ObjectInitializer)
{
	LocomotionBSDirection = 0.0f;
	bLocomotionMovingForward = true;
	MoveSpeed = 0.0f;
	bIsMoving = false;

	BlendSpaceDefaultMoveSpeed = 600.0f;
	LocomotionPlayRate = 1.0f;

	bIsInAir = false;
	bDoubleJumped = false;
	JumpCount = 0;
}


void URPGAnimInstanceBase::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	OwnerCharacter = Cast<ACharacter>(GetOwningActor());
}

void URPGAnimInstanceBase::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	UpdateLocomotionVars(DeltaSeconds);
}

void URPGAnimInstanceBase::UpdateLocomotionVars(float DeltaSeconds)
{
	if (!OwnerCharacter)
	{
		return;
	}

	FVector Velocity = GetOwningActor()->GetVelocity();
	
	MoveSpeed = Velocity.Size();
	LocomotionPlayRate = MoveSpeed * (1.0f / BlendSpaceDefaultMoveSpeed);

	bIsMoving = MoveSpeed > 0.0f;

	FRotator VelocityRot = Velocity.ToOrientationRotator();
	LocomotionBSDirection = (bIsMoving) ? (VelocityRot - GetOwningActor()->GetActorRotation()).GetNormalized().Yaw : 0.0f;
	if (bLocomotionMovingForward && (LocomotionBSDirection < -95.0f || LocomotionBSDirection > 95.0f))
	{
		bLocomotionMovingForward = false;
	}
	else if (!bLocomotionMovingForward && (LocomotionBSDirection > -90.0f && LocomotionBSDirection < 90.0f))
	{
		bLocomotionMovingForward = true;
	}

	//if we are moving backward then we need to map it to the 360 range so it is a smooth animations
	LocomotionBSDirection = (!bLocomotionMovingForward) ? FMath::Fmod(360 + LocomotionBSDirection, 360) : LocomotionBSDirection;

	//if we haven't jumped before and we just just jumped now or we started falling then play the first jump animations
	//if we didn't first jump and we haven't double jumped
	//bool bJumped = !bIsInAir && OwnerCharacter->GetMovementComponent()->IsFalling() || NewJumpCount > JumpCount;
	bIsInAir = OwnerCharacter->GetMovementComponent()->IsFalling();

	//b double jump if we haven't new jump count is greater than 1
	const int32 NewJumpCount = OwnerCharacter->JumpCurrentCount;
	bDoubleJumped = NewJumpCount > JumpCount && NewJumpCount > 1;
	JumpCount = NewJumpCount;
}

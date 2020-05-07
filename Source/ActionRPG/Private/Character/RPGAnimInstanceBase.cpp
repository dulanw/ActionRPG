// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/RPGAnimInstanceBase.h"
#include "Character/RPGCharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"

URPGAnimInstanceBase::URPGAnimInstanceBase(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	: Super(ObjectInitializer)
{
	//MoveAngle = 0.0f;
	//bIsMovingForward = true;

	LocomotionBlendSpaceDirection = 0.0f;
	LocomotionRootRotation = 0.0f;
	BlendLocomotionRootRotation = 1.0f;

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
	LocomotionBlendSpaceDirection = (bIsMoving) ? (VelocityRot - GetOwningActor()->GetActorRotation()).GetNormalized().Yaw : 0.0f;

/*
	//FRotator VelocityRot = FRotationMatrix::MakeFromX(Velocity).Rotator().GetNormalized();
	//#TODO fix issue with 0 velocity
	FRotator VelocityRot = Velocity.ToOrientationRotator();
	//different between the orientation of the actor and the velocity
	float VelocityOrientDiffAngle = (int) (VelocityRot - GetOwningActor()->GetActorRotation()).GetNormalized().Yaw;	 	// (int) stop annoying round errors
	DebugMoveDirection = VelocityOrientDiffAngle; 	//get the reminder when divided by 90
	DebugAngleRemainder = FMath::CeilToFloat(VelocityOrientDiffAngle / 90.0f);

	//if we are less than negative then we round down to the nearest integer or round up
	LocomotionBlendSpaceDirection = FMath::CeilToFloat(VelocityOrientDiffAngle / 90.0f) * 90.0f;
	LocomotionRootRotation = VelocityOrientDiffAngle - LocomotionBlendSpaceDirection;*/

	//if (bIsMovingForward && (MoveAngle < -95.0f || MoveAngle > 95.0f))
	//{
	//	bIsMovingForward = false;
	//}
	//else if (!bIsMovingForward && (MoveAngle > -92.5f && MoveAngle < 92.5f))
	//{
	//	bIsMovingForward = true;
	//}

	//float TempMoveAngle = MoveAngle;
	////if we are moving backward then we need to map the 
	//MoveAngle = (!bIsMovingForward) ? FMath::Fmod(360 + MoveAngle, 360) : MoveAngle;

	//if we haven't jumped before and we just just jumped now or we started falling then play the first jump animations
	//if we didn't first jump and we haven't double jumped
	//bool bJumped = !bIsInAir && OwnerCharacter->GetMovementComponent()->IsFalling() || NewJumpCount > JumpCount;
	bIsInAir = OwnerCharacter->GetMovementComponent()->IsFalling();

	//b double jump if we haven't new jump count is greater than 1
	const int32 NewJumpCount = OwnerCharacter->JumpCurrentCount;
	bDoubleJumped = NewJumpCount > JumpCount && NewJumpCount > 1;
	JumpCount = NewJumpCount;
}

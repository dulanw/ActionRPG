// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/RPGPathFollowingComponent.h"
#include "AI/RPGAIController.h"
#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h"

URPGPathFollowingComponent::URPGPathFollowingComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	
}

void URPGPathFollowingComponent::FollowPathSegment(float DeltaTime)
{
	Super::FollowPathSegment(DeltaTime);

/*
	const TArray<FNavPathPoint>& NavPath = Path->GetPathPoints();
	for (int32 i = 0; i < NavPath.Num(); i++)
	{
		FNavPathPoint NavPoint = NavPath[i];
		DrawDebugSphere(GetWorld(), NavPoint.Location, 5.0f, 4, FColor::Red, false, DeltaTime, 0, 2.0f);

		if (i > 0)
		{
			FNavPathPoint PrevPoint = NavPath[i - 1];
			DrawDebugLine(GetWorld(), NavPoint.Location, PrevPoint.Location, FColor::Red, false, DeltaTime, 0, 2.0f);
		}
	}*/

/*
	if (Path->IsPartial())
	{
		OnPathFinished(EPathFollowingResult::Blocked, FPathFollowingResultFlags::None);
		return;
	}*/

/*
	//const ANavigationData* NavData = GetNavData();
	TArray<NavNodeRef> NavPolys;
	if (GetAllPolys(NavPolys))
	{
		//UE_LOG(LogTemp, Warning, TEXT("Found %d NavPolys"), NavPolys.Num());
		for (NavNodeRef NavPoly : NavPolys)
		{
			FBox Bounds = NavPoly_GetBounds(NavPoly);

			FVector Center(0.0f);
			FVector Extent(0.0f);
			Bounds.GetCenterAndExtents(Center, Extent);

			//FVector Center = NavPoly_GetCenter(NavPoly);
			if (Path->ContainsNode(NavPoly))
			{
				DrawDebugBox(GetWorld(), Center, Extent, FQuat::Identity, NavPoly == MoveSegmentStartRef ? FColor::Green : NavPoly == MoveSegmentEndRef ? FColor::Red : FColor::Blue, false, DeltaTime, 0, 2.5f);
			}
			else
			{
				DrawDebugBox(GetWorld(), Center, Extent, FQuat::Identity, FColor::Yellow, false, DeltaTime, 0, 2.5f);
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No Nav Data"));
	}*/

/*
	TArray<NavNodeRef> NavPolys;
	if (GetAllPolys(NavPolys))
	{
		//UE_LOG(LogTemp, Warning, TEXT("Found %d NavPolys"), NavPolys.Num());
		for (NavNodeRef NavPoly : NavPolys)
		{
			FBox Bounds = NavPoly_GetBounds(NavPoly);

			FVector Center(0.0f);
			FVector Extent(0.0f);
			Bounds.GetCenterAndExtents(Center, Extent);

			DrawDebugBox(GetWorld(), Center, Extent, FQuat::Identity, FColor::Yellow, false, DeltaTime, 0, 2.0f);

			////FVector Center = NavPoly_GetCenter(NavPoly);
			//if (Path->ContainsNode(NavPoly))
			//{
			//	DrawDebugBox(GetWorld(), Center, Extent, FQuat::Identity, NavPoly == MoveSegmentStartRef ? FColor::Green : NavPoly == MoveSegmentEndRef ? FColor::Red : FColor::Blue, false, DeltaTime, 0, 2.5f);
			//}
			//else
			//{
			//	DrawDebugBox(GetWorld(), Center, Extent, FQuat::Identity, FColor::Yellow, false, DeltaTime, 0, 2.5f);
			//}
		}
	}*/

}

bool URPGPathFollowingComponent::GetAllPolys(TArray<NavNodeRef>& OutPolys)
{
	if (!MovementComp)
	{
		return false;
	}

	//Get Nav Data
	const ANavigationData* NavData = GetNavData();

	const ARecastNavMesh* NavMesh = Cast<ARecastNavMesh>(NavData);
	if (!NavMesh)
	{
		return false;
	}

	TArray<FNavPoly> EachPolys;
	for (int32 v = 0; v < NavMesh->GetNavMeshTilesCount(); v++)
	{

		// 256 entries but only few are valid!
		// use continue in case the valid polys are not stored sequentially
		if (!TileIsValid(NavMesh, v))
		{
			continue;
		}

		NavMesh->GetPolysInTile(v, EachPolys);
	}


	//Add them all!
	for (int32 v = 0; v < EachPolys.Num(); v++)
	{
		OutPolys.Add(EachPolys[v].Ref);
	}

	return true;
}

bool URPGPathFollowingComponent::NavPoly_GetVerts(const NavNodeRef& PolyID, TArray<FVector>& OutVerts) const
{
	//Get Nav Data
	const ANavigationData* NavData = GetNavData();

	const ARecastNavMesh* NavMesh = Cast<ARecastNavMesh>(NavData);
	if (!NavMesh)
	{
		return false;
	}

	return NavMesh->GetPolyVerts(PolyID, OutVerts);
}

FBox URPGPathFollowingComponent::NavPoly_GetBounds(const NavNodeRef& PolyID) const
{
	TArray<FVector> Verts;
	NavPoly_GetVerts(PolyID, Verts); //#TODO check return bool 

	FBox Bounds(EForceInit::ForceInitToZero);
	for (const FVector& Each : Verts)
	{
		Bounds += Verts;
	}

	return Bounds;
}

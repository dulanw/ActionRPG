// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationSystem.h"
#include "NavMesh/RecastNavMesh.h"
#include "RPGPathFollowingComponent.generated.h"

/**
 * #TODO pass in the nav data to the getpoly verts etc.. 
 * #TODO override OnPathFinished and check FPathFollowingResultFlags::Blocked
 */
UCLASS()
class ACTIONRPG_API URPGPathFollowingComponent : public UPathFollowingComponent
{
	GENERATED_BODY()

public:
	URPGPathFollowingComponent(const FObjectInitializer& ObjectInitializer);

protected:
	/** follow current path segment */
	virtual void FollowPathSegment(float DeltaTime) override;

	/**
	 * Get main Nav Data, called by GetNavData if MovementComponent or Nav Data on NavAgent on MovementComponent is not found
	 */
	FORCEINLINE const ANavigationData* GetMainNavData() const
	{
		const UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld()); //can't call this cause of the const

		if (!NavSys)
		{
			return nullptr;
		}

		/** GetMainNavData is deprecated
		 *	If you're using NavigationSysstem module consider calling
		 *	FNavigationSystem::GetCurrent<UNavigationSystemV1>()->GetDefaultNavDataInstance
		 *	instead.
		 */
		return NavSys->GetDefaultNavDataInstance();
	}

	/**
	 * Get the nav data for the agent
	 */
	FORCEINLINE const ANavigationData* GetNavData() const
	{
		if (!MovementComp)
		{
			return GetMainNavData();
		}

		const UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld()); //can't call this cause of the const
		if (!NavSys)
		{
			return nullptr;
		}

		const FNavAgentProperties& AgentProperties = MovementComp->GetNavAgentPropertiesRef();
		const ANavigationData* NavData = NavSys->GetNavDataForProps(AgentProperties);
		if (!NavData)
		{
			UE_LOG(LogTemp, Error, TEXT("Error Using Main Nav Data"));
			NavData = GetMainNavData();
		}

		return NavData;
	}

	/**
	 * very important for crash protection
	 */
	FORCEINLINE bool TileIsValid(const ARecastNavMesh* NavMesh, int32 TileIndex) const
	{
		if (!NavMesh)
		{
			return false;
		}

		const FBox TileBounds = NavMesh->GetNavMeshTileBounds(TileIndex);

		return TileBounds.IsValid != 0;
	}

	//#TODO pass in nav data cast Cast<ARecastNavMesh>(NavData) so we can call NavPoly_GetLocation etc.. without having to call GetnavData over and over again 
	bool GetAllPolys(TArray<NavNodeRef>& OutPolys);

	//Verts
	bool NavPoly_GetVerts(const NavNodeRef& PolyID, TArray<FVector>& OutVerts) const;

	//Bounds
	FBox NavPoly_GetBounds(const NavNodeRef& PolyID) const;
};

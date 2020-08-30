// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NavMesh/RecastNavMesh.h"
#include "RPGRecastNavMesh.generated.h"

/**
 * 
 */
UCLASS()
class ACTIONRPG_API ARPGRecastNavMesh : public ARecastNavMesh
{
	GENERATED_BODY()

public:
	ARPGRecastNavMesh(const FObjectInitializer& ObjectInitializer);
	static FPathFindingResult FindPath(const FNavAgentProperties& AgentProperties, const FPathFindingQuery& Query);

};

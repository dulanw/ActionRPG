// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayAbilitySpec.h"
#include "RPGAIBlueprintHelperLibrary.generated.h"

/**
 * 
 */
UCLASS()
class ACTIONRPG_API URPGAIBlueprintHelperLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "AI Blueprint Helper Library")
	static void GetNearestPlayerPawn(class AActor* const &Querier, class APawn* &OutNearestPlayerPawn, float &OutDistance);

	/**
	 * Get the first actor from the query result, c++ version that calls GetQueryResultsAsActors::GetQueryResultsAsActors and returns the first valid item
	 * @return null if query still processing or it failed or there are no actors to be found
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Blueprint Helper Library")
	static class AActor* GetQueryResultsAsActor(const class UEnvQueryInstanceBlueprintWrapper* const &Query);

	UFUNCTION(BlueprintCallable, Category = "AI Blueprint Helper Library")
	static FGameplayAbilitySpecHandle GiveAbility(class UAbilitySystemComponent* const &AbilitySystemComponent, const TSubclassOf<class UGameplayAbility> &InAbility, int32 InLevel = 1, UObject* InSourceObject = nullptr);

	/** Returns the radius of the collision cylinder from GetSimpleCollisionCylinder(). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AI Blueprint Helper Library")
	static float GetSimpleCollisionRadius(const class AActor* const &Actor);
};

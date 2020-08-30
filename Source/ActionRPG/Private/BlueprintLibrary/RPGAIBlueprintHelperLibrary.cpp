// Fill out your copyright notice in the Description page of Project Settings.


#include "BlueprintLibrary/RPGAIBlueprintHelperLibrary.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/Pawn.h"
#include "EnvironmentQuery/EnvQueryInstanceBlueprintWrapper.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"

void URPGAIBlueprintHelperLibrary::GetNearestPlayerPawn(AActor* const &Querier, class APawn*& OutNearestPlayerPawn, float& OutDistance)
{
	if (!Querier)
	{
		OutNearestPlayerPawn = nullptr;
		OutDistance = -1.0f;

		return;
	}

	TArray<APlayerState*> PlayerArray = Querier->GetWorld()->GetGameState()->PlayerArray;

	APawn* NearestPawn = nullptr;
	float MinDistance = -1.0f;

	for (int32 Index = 0; Index < PlayerArray.Num(); Index++)
	{
		APlayerState* PlayerState = PlayerArray[Index];
		APawn* Pawn = PlayerState->GetPawn();

		if (!PlayerState->IsSpectator() && !PlayerState->IsOnlyASpectator())
		{
			float Distance = (Querier->GetActorLocation() - Pawn->GetActorLocation()).Size();

			if (NearestPawn) // if we already run this, we can't use index = 0 since the first pawn might be a spectator
			{
				if (Distance < MinDistance)
				{
					NearestPawn = Pawn;
					MinDistance = Distance;
				}
			}
			else
			{
				NearestPawn = Pawn;
				MinDistance = Distance;
			}
		}
	}

	OutNearestPlayerPawn = NearestPawn;
	OutDistance = MinDistance;
}

AActor* URPGAIBlueprintHelperLibrary::GetQueryResultsAsActor(const UEnvQueryInstanceBlueprintWrapper* const& Query)
{
	TArray<AActor*> ResultActors;
	if (Query->GetQueryResultsAsActors(ResultActors))
	{
		if (ResultActors.Num() > 0)
		{
			return ResultActors[0];
		}
	}

	return nullptr;
}

FGameplayAbilitySpecHandle URPGAIBlueprintHelperLibrary::GiveAbility(UAbilitySystemComponent* const& AbilitySystemComponent, const TSubclassOf<UGameplayAbility>& InAbility, int32 InLevel /*= 1*/, UObject* InSourceObject /*= nullptr*/)
{
	check(AbilitySystemComponent);
	check(InAbility);

	return AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(InAbility, InLevel, INDEX_NONE, InSourceObject));
}

float URPGAIBlueprintHelperLibrary::GetSimpleCollisionRadius(const AActor* const& Actor)
{
	return Actor->GetSimpleCollisionRadius();
}

bool URPGAIBlueprintHelperLibrary::TryActivateAbility(class UAbilitySystemComponent* const& AbilitySystemComponent, FGameplayAbilitySpecHandle AbilityToActivate, bool bAllowRemoteActivation /*= true*/)
{
	check(AbilitySystemComponent);

	return AbilitySystemComponent->TryActivateAbility(AbilityToActivate, bAllowRemoteActivation);
}

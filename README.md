# ActionRPG
Dungeon Crawler coop action RPG, using the new unreal engine gameplay ability system (same one used in fortnite BR) with support for networking and AI.

## Ability System Component
The ability system component lives on the player/AI pawn, although it could be on the player state as well but this is done to cut down the work on having it in 2 different places since AI don't have a player state.

Need to call `AbilitySystemComponent->InitAbilityActorInfo(OwnerActor, AvatarActor);`
this needs to be called on both the server and the client, currently this is all done in the ARPGCharacterBase by overriding `PossessedBy(AController * NewController)` and  `OnRep_Controller()`
It needs to be called in 2 places since `PossessedBy(AController * NewController)` will only be called on the server and `OnRep_Controller()` is for the client, it can also be called on the player controller `AcknowledgePossession(APawn* P)` instead of `OnRep_Controller`

## Abilities

#TODO

## Inventory Slots
#TODO

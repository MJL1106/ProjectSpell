// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/AuraSummonAbility.h"

#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "Character/AuraEnemy.h"


TArray<FVector> UAuraSummonAbility::GetSpawnLocations()
{
	const FVector Forward = GetAvatarActorFromActorInfo()->GetActorForwardVector();
	const FVector Location = GetAvatarActorFromActorInfo()->GetActorLocation();
	// Calculate the angle between each minion (total spread divided by number of minions)
	const float DeltaSpread = SpawnSpread / NumMinions;

	// Calculate the leftmost direction of the spawn spread
	const FVector LeftOfSpread = Forward.RotateAngleAxis(-SpawnSpread/2.f, FVector::UpVector);
	TArray<FVector> SpawnLocations;
	for (int32 i = 0; i < NumMinions; i++)
	{
		const FVector Direction = LeftOfSpread.RotateAngleAxis(DeltaSpread*i, FVector::UpVector);
		
		// Choose a random distance between the min and max spawn distance. Then multiply by the direction to get the actual position vector
		FVector ChosenSpawnLocation = Location + Direction * FMath::FRandRange(MinSpawnDistance, MaxSpawnDistance);

		// Checks if there's ground surface to place the minion on
		FHitResult Hit;
		GetWorld()->LineTraceSingleByChannel(Hit, ChosenSpawnLocation + FVector(0.f, 0.f, 400.f), ChosenSpawnLocation - FVector(0.f, 0.f, 400.f), ECC_Visibility);
		if (Hit.bBlockingHit)
		{
			// Adjusts the spawn location to be the exact point that the line trace hits the surface
			ChosenSpawnLocation = Hit.ImpactPoint;
		}
		SpawnLocations.Add(ChosenSpawnLocation);
	}
	return SpawnLocations;
}

TSubclassOf<APawn> UAuraSummonAbility::GetRandomMinionClass()
{
	const int32 Selection = FMath::RandRange(0, MinionClasses.Num() - 1);
	return MinionClasses[Selection];
}

APawn* UAuraSummonAbility::SpawnSummonEnemies(TSubclassOf<APawn> MinionClass, const FVector& SpawnLocation, const FRotator& SpawnRotation)
{
	if (!MinionClass)
	{
		return nullptr;
	}

	UAbilitySystemComponent* ASC = GetAvatarActorFromActorInfo()->FindComponentByClass<UAbilitySystemComponent>();
	if (!ASC)
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	FTransform SpawnTransform(SpawnRotation, SpawnLocation);
	
	AAuraEnemy* Enemy = GetWorld()->SpawnActorDeferred<AAuraEnemy>(MinionClass, SpawnTransform);
	Enemy->SetLevel(ICombatInterface::Execute_GetPlayerLevel(ASC->GetAvatarActor()));
	Enemy->FinishSpawning(SpawnTransform);
	Enemy->SpawnDefaultController();
	
	return Cast<APawn>(Enemy);
}

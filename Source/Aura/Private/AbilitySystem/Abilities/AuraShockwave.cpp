// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/AuraShockwave.h"

#include "AbilitySystem/AuraAbilitySystemLibrary.h"

TArray<AAuraShockBolt*> UAuraShockwave::SpawnShockwave()
{
	TArray<AAuraShockBolt*> Shockwaves;
	const FVector Forward = GetAvatarActorFromActorInfo()->GetActorForwardVector();
	const FVector Location = GetAvatarActorFromActorInfo()->GetActorLocation();
	TArray<FRotator> Rotators = UAuraAbilitySystemLibrary::EvenlySpacedRotators(Forward, FVector::UpVector, 360.f, NumShockBolts);

	for (const FRotator& Rotator : Rotators)
	{
		FTransform SpawnTransform;
		SpawnTransform.SetLocation(Location);
		SpawnTransform.SetRotation(Rotator.Quaternion());
		AAuraShockBolt* ShockBolt = GetWorld()->SpawnActorDeferred<AAuraShockBolt>(
			ShockBoltClass,
			SpawnTransform,
			GetOwningActorFromActorInfo(),
			CurrentActorInfo->PlayerController->GetPawn(),
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

		ShockBolt->DamageEffectParams = MakeDamageEffectParamsFromClassDefaults();

		ShockBolt->ExplosionDamageParams = MakeDamageEffectParamsFromClassDefaults();
		ShockBolt->SetOwner(GetAvatarActorFromActorInfo());
		
		Shockwaves.Add(ShockBolt);
		ShockBolt->FinishSpawning(SpawnTransform);
	}
	return Shockwaves;
}

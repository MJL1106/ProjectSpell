// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/AuraFireTornado.h"

#include "Actor/AuraProjectile.h"

void UAuraFireTornado::SpawnTornado(const FVector& ProjectileTargetLocation, const FGameplayTag& SocketTag, bool bOverridePitch, float PitchOverride)
{
	const bool bIsServer = GetAvatarActorFromActorInfo()->HasAuthority();
	if (!bIsServer) return;

	// Get the socket location where the projectile will be spawned
	const FVector SocketLocation = ICombatInterface::Execute_GetCombatSocketLocation(
		GetAvatarActorFromActorInfo(),
		SocketTag);
    
	// Calculate the direction from the socket location to the target location
	FVector Direction = (ProjectileTargetLocation - SocketLocation).GetSafeNormal2D();  // Use 2D to ignore Z-axis (height)
    
	// Set rotation based on the direction, but lock pitch to 0 (move along the ground)
	FRotator Rotation = Direction.Rotation();
	Rotation.Pitch = 0.0f;  // Keep the projectile moving horizontally

	// If pitch override is provided, you can still allow an override
	if (bOverridePitch)
	{
		Rotation.Pitch = PitchOverride;
	}

	// Set the spawn transform (position and rotation)
	FTransform SpawnTransform;
	SpawnTransform.SetLocation(SocketLocation);
	SpawnTransform.SetRotation(Rotation.Quaternion());

	// Spawn the projectile
	AAuraProjectile* Projectile = GetWorld()->SpawnActorDeferred<AAuraProjectile>(
		ProjectileClass,
		SpawnTransform,
		GetOwningActorFromActorInfo(),
		Cast<APawn>(GetOwningActorFromActorInfo()),
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn
	);

	// Apply damage parameters
	Projectile->DamageEffectParams = MakeDamageEffectParamsFromClassDefaults();
    
	// Finish the spawning process
	Projectile->FinishSpawning(SpawnTransform);
}

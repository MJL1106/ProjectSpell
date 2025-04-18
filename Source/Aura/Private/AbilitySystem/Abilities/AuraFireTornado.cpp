// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/AuraFireTornado.h"

#include "Actor/AuraProjectile.h"

FString UAuraFireTornado::GetDescription(int32 Level)
{
	const int32 ScaledDamage = Damage.GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);
	return FString::Printf(TEXT(
			"<Title>FIRE TORNADO</>\n\n"

			"<Small>Level: </><Level>%d</>\n"
			"<Small>ManaCost: </><ManaCost>%.1f</>\n"
			"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"

			"<Default>Summons fire tornado, "
			"sweeping enemies off their feet causing them to repeatedly burn, with a damage of "
			"</><Damage>%d</><Default>.</>"
			),
			Level,
			ManaCost,
			Cooldown,
			ScaledDamage);
}

FString UAuraFireTornado::GetNextLevelDescription(int32 Level)
{
	const int32 ScaledDamage = Damage.GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);
	return FString::Printf(TEXT(
			"<Title>NEXT LEVEL</>\n\n"

			"<Small>Level: </><Level>%d</>\n"
			"<Small>ManaCost: </><ManaCost>%.1f</>\n"
			"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"

			"<Default>Summons fire tornado, "
			"sweeping enemies off their feet causing them to repeatedly burn, with a damage of "
			"</><Damage>%d</><Default>.</>"
			),
			Level,
			ManaCost,
			Cooldown,
			ScaledDamage);
}

void UAuraFireTornado::SpawnTornado(const FVector& ProjectileTargetLocation, const FGameplayTag& SocketTag, bool bOverridePitch, float PitchOverride)
{
	const bool bIsServer = GetAvatarActorFromActorInfo()->HasAuthority();
	if (!bIsServer) return;
	
	const FVector SocketLocation = ICombatInterface::Execute_GetCombatSocketLocation(
		GetAvatarActorFromActorInfo(),
		SocketTag);
	
    FVector GroundLocation = SocketLocation;
    FHitResult HitResult;
    FVector TraceStart = SocketLocation;
    FVector TraceEnd = SocketLocation - FVector(0, 0, 1000);

    FCollisionQueryParams TraceParams(FName(TEXT("GroundTrace")), false, GetAvatarActorFromActorInfo());
    if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, TraceParams))
    {
        GroundLocation.Z = HitResult.Location.Z;
    }
    FVector Direction = (ProjectileTargetLocation - GroundLocation).GetSafeNormal2D();
	
    FRotator Rotation = Direction.Rotation();
    Rotation.Pitch = 0.0f;

    if (bOverridePitch)
    {
        Rotation.Pitch = PitchOverride;
    }

    FTransform SpawnTransform;
    FVector SpawnLocation = SocketLocation;
    SpawnLocation.Z = GroundLocation.Z + 10.f;
    SpawnTransform.SetLocation(SpawnLocation);
    SpawnTransform.SetRotation(Rotation.Quaternion());
	
    AAuraProjectile* Projectile = GetWorld()->SpawnActorDeferred<AAuraProjectile>(
        ProjectileClass,
        SpawnTransform,
        GetOwningActorFromActorInfo(),
        Cast<APawn>(GetOwningActorFromActorInfo()),
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn
    );

    Projectile->DamageEffectParams = MakeDamageEffectParamsFromClassDefaults();
    Projectile->FinishSpawning(SpawnTransform);
}

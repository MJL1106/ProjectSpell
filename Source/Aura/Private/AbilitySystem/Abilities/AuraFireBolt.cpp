// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/AuraFireBolt.h"

#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Actor/AuraProjectile.h"
#include "Aura/Public/AuraGameplayTags.h"

FString UAuraFireBolt::GetDescription(int32 Level)
{
	const int32 ScaledDamage = Damage.GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);
	if (Level == 1)
	{
		return FString::Printf(TEXT(
			"<Title>FIRE BOLT</>\n\n"

			"<Small>Level: </><Level>%d</>\n"
			"<Small>ManaCost: </><ManaCost>%.1f</>\n"
			"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"

			"<Default>Launches a bolt of fire, "
			"exploding on impact and dealing: "
			"</><Damage>%d</><Default> fire damage with a chance to burn</>"
			),
			Level,
			ManaCost,
			Cooldown,
			ScaledDamage);
	}
	else
	{
		return FString::Printf(TEXT(
		"<Title>FIRE BOLT</>\n\n"

		"<Small>Level: </><Level>%d</>\n"
		"<Small>ManaCost: </><ManaCost>%.1f</>\n"
		"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"

		"<Default>Launches %d bolts of fire, "
		"exploding on impact and dealing: </>"
		
		"<Damage>%d</><Default> fire damage with a chance to burn</>"
		),
		Level,
		ManaCost,
		Cooldown,
		FMath::Min(Level, NumProjectiles),
		ScaledDamage);
	}
}

FString UAuraFireBolt::GetNextLevelDescription(int32 Level)
{
	const int32 ScaledDamage = Damage.GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);
	return FString::Printf(TEXT(
	"<Title>NEXT LEVEL: </>\n\n"

	"<Small>Level: </><Level>%d</>\n"
	"<Small>ManaCost: </><ManaCost>%.1f</>\n"
	"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"

	"<Default>Launches %d bolts of fire, "
	"exploding on impact and dealing: </>"
			
	"<Damage>%d</><Default> fire damage with a chance to burn</>"
	),
	Level,
	ManaCost,
	Cooldown,
	FMath::Min(Level, NumProjectiles),
	ScaledDamage);	
}

void UAuraFireBolt::SpawnProjectiles(const FVector& ProjectileTargetLocation, const FGameplayTag& SocketTag, bool bOverridePitch, float PitchOverride,
	AActor* HomingTarget)
{
	const bool bIsServer = GetAvatarActorFromActorInfo()->HasAuthority();
	if (!bIsServer) return;

	const FVector SocketLocation = ICombatInterface::Execute_GetCombatSocketLocation(
		GetAvatarActorFromActorInfo(),
		SocketTag
		);
	
	FRotator Rotation = (ProjectileTargetLocation - SocketLocation).Rotation();
	if (bOverridePitch)
	{
		Rotation.Pitch = PitchOverride;
	}

	const FVector Forward = Rotation.Vector();

	TArray<FRotator> Rotations = UAuraAbilitySystemLibrary::EvenlySpacedRotators(Forward, FVector::UpVector, ProjectileSpread, NumProjectiles);

	for (const FRotator& Rot : Rotations)
	{
		FTransform SpawnTransform;
		SpawnTransform.SetLocation(SocketLocation);
		SpawnTransform.SetRotation(Rot.Quaternion());

		AAuraProjectile* Projectile = GetWorld()->SpawnActorDeferred<AAuraProjectile>(
		ProjectileClass,
		SpawnTransform,
		GetOwningActorFromActorInfo(),
		Cast<APawn>(GetOwningActorFromActorInfo()),
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		
		Projectile->DamageEffectParams = MakeDamageEffectParamsFromClassDefaults();
		Projectile->FinishSpawning(SpawnTransform);
	}
}

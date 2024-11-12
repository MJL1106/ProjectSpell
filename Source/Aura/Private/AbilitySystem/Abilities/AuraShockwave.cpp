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

FString UAuraShockwave::GetDescription(int32 Level)
{
	const int32 ScaledDamage = Damage.GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);
	return FString::Printf(TEXT(
			"<Title>SHOCKWAVE</>\n\n"

			"<Small>Level: </><Level>%d</>\n"
			"<Small>ManaCost: </><ManaCost>%.1f</>\n"
			"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"

			"<Default>Shoots a circular wave of bolts towards enemies, "
			"causing damage of "
			"</><Damage>%d</><Default> to any enemy hit directly or on explosion.</>"
			),
			Level,
			ManaCost,
			Cooldown,
			ScaledDamage);
}

FString UAuraShockwave::GetNextLevelDescription(int32 Level)
{
	const int32 ScaledDamage = Damage.GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);
	return FString::Printf(TEXT(
			"<Title>NEXT LEVEL</>\n\n"

			"<Small>Level: </><Level>%d</>\n"
			"<Small>ManaCost: </><ManaCost>%.1f</>\n"
			"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"

			"<Default>Shoots a circular wave of bolts towards enemies, "
			"causing damage of "
			"</><Damage>%d</><Default> to any enemy hit directly or on explosion.</>"
			),
			Level,
			ManaCost,
			Cooldown,
			ScaledDamage);
}

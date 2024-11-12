// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/AuraArcaneVortex.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "AuraAbilityTypes.h"
#include "TimerManager.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"

FString UAuraArcaneVortex::GetDescription(int32 Level)
{
	const int32 ScaledDamage = Damage.GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);
	return FString::Printf(TEXT(
			"<Title>ARCANE VORTEX</>\n\n"

			"<Small>Level: </><Level>%d</>\n"
			"<Small>ManaCost: </><ManaCost>%.1f</>\n"
			"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"

			"<Default>Summons a vortex of arcane energy, "
			"pulling in any enemies within it's gravitational area, dealing "
			"</><Damage>%d</><Default> to any enemy warped in.</>"
			),
			Level,
			ManaCost,
			Cooldown,
			ScaledDamage);
}

FString UAuraArcaneVortex::GetNextLevelDescription(int32 Level)
{
	const int32 ScaledDamage = Damage.GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);
	return FString::Printf(TEXT(
			"<Title>NEXT LEVEL</>\n\n"

			"<Small>Level: </><Level>%d</>\n"
			"<Small>ManaCost: </><ManaCost>%.1f</>\n"
			"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"

			"<Default>Summons a vortex of arcane energy, "
			"pulling in any enemies within it's gravitational area, dealing "
			"</><Damage>%d</><Default> to any enemy warped in.</>"
			),
			Level,
			ManaCost,
			Cooldown,
			ScaledDamage);
}

void UAuraArcaneVortex::PullActorTowardsLocation(AActor* ActorToPull, const FVector& TargetLocation, float PullSpeed)
{
	if (!ActorToPull) return;

	FVector CurrentLocation = ActorToPull->GetActorLocation();
	FVector ModifiedTargetLocation = TargetLocation;
	ModifiedTargetLocation.Z = CurrentLocation.Z;

	FVector NewLocation = FMath::VInterpTo(CurrentLocation, ModifiedTargetLocation, GetWorld()->GetDeltaSeconds(), PullSpeed);

	float DistanceToTarget = FVector::Dist2D(CurrentLocation, ModifiedTargetLocation);

	NewLocation = ModifiedTargetLocation;

	ACharacter* Character = Cast<ACharacter>(ActorToPull);
	if (Character && Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->DisableMovement();
		
		DamageEffectParams = MakeDamageEffectParamsFromClassDefaults();
		
		FTimerHandle HoldTimerHandle;
		GetWorld()->GetTimerManager().SetTimer(HoldTimerHandle, [Character, ActorToPull, this]()
		{
			if (Character && Character->GetCharacterMovement())
			{
				Character->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
				
				if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(ActorToPull))
				{
					DamageEffectParams.TargetAbilitySystemComponent = TargetASC;
					UAuraAbilitySystemLibrary::ApplyDamageEffect(DamageEffectParams);
				}
			}
		}, 0.8f, false);
	}
	
	ActorToPull->SetActorLocation(NewLocation, true);
}

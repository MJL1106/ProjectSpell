// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/AuraArcaneVortex.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "AuraAbilityTypes.h"
#include "TimerManager.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"

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

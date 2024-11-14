// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/AuraDamageGameplayAbility.h"
#include "AuraArcaneVortex.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API UAuraArcaneVortex : public UAuraDamageGameplayAbility
{
	GENERATED_BODY()
public:
	virtual FString GetDescription(int32 Level) override;
	virtual FString GetNextLevelDescription(int32 Level) override;
	
	UFUNCTION(BlueprintCallable, Category = "Ability")
	void PullActorTowardsLocation(AActor* ActorToPull, const FVector& TargetLocation, float PullSpeed);

	UPROPERTY(BlueprintReadWrite, meta = (ExposeOnSpawn = true))
	FDamageEffectParams DamageEffectParams;
};

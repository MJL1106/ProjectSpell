// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/AuraProjectileSpell.h"
#include "AuraFireTornado.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API UAuraFireTornado : public UAuraProjectileSpell
{
	GENERATED_BODY()

protected:

	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void SpawnTornado(const FVector& ProjectileTargetLocation, const FGameplayTag& SocketTag, bool bOverridePitch = false, float PitchOverride = 0.f);
};

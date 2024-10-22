// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/AuraDamageGameplayAbility.h"
#include "Actor/AuraShockBolt.h"
#include "AuraShockwave.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API UAuraShockwave : public UAuraDamageGameplayAbility
{
	GENERATED_BODY()
public:
	
	UFUNCTION(BlueprintCallable)
	TArray<AAuraShockBolt*> SpawnShockwave();

protected:

	UPROPERTY(EditDefaultsOnly, Category = "Shockwave")
	int32 NumShockBolts = 12;

private:

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AAuraShockBolt> ShockBoltClass;
};

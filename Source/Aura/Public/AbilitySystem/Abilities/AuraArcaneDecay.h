// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/AuraDamageGameplayAbility.h"
#include "AuraArcaneDecay.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API UAuraArcaneDecay : public UAuraDamageGameplayAbility
{
	GENERATED_BODY()
public:
    virtual FString GetDescription(int32 Level) override;
    virtual FString GetNextLevelDescription(int32 Level) override;
};

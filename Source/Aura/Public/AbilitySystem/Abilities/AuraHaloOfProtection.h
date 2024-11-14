// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/AuraPassiveAbility.h"
#include "AuraHaloOfProtection.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API UAuraHaloOfProtection : public UAuraPassiveAbility
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = "PassiveAbilities|HaloOfProtection")
	UCurveFloat* HaloOfProtectionCurve;
	
	virtual FString GetDescription(int32 Level) override;
	virtual FString GetNextLevelDescription(int32 Level) override;
};

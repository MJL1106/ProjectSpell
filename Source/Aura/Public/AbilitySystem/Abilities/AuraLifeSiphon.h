// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/AuraPassiveAbility.h"
#include "AuraLifeSiphon.generated.h"

class AAuraCharacterBase;
/**
 * 
 */
UCLASS()
class AURA_API UAuraLifeSiphon : public UAuraPassiveAbility
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = "PassiveAbilities|LifeSiphon")
	UCurveFloat* LifeSiphonCurve;
	
	virtual FString GetDescription(int32 Level) override;
	virtual FString GetNextLevelDescription(int32 Level) override;
};

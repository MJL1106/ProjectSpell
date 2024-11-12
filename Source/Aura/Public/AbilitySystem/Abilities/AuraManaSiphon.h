// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/AuraPassiveAbility.h"
#include "AuraManaSiphon.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API UAuraManaSiphon : public UAuraPassiveAbility
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = "PassiveAbilities|ManaSiphon")
	UCurveFloat* ManaSiphonCurve;
	
	virtual FString GetDescription(int32 Level) override;
	virtual FString GetNextLevelDescription(int32 Level) override;
};
